#!/usr/bin/env bash
#
# render_movie.sh — Render a sequence of frames sweeping a scene parameter
# from a min to a max value, then assemble them into a movie with ffmpeg.
#
# Usage:
#   ./render_movie.sh --param <name> --min <float> --max <float> [options]
#
# Required:
#   --param <name>       Scene parameter name (passed as --scene-param name=value)
#   --min <float>        Starting value for the parameter
#   --max <float>        Ending value for the parameter
#
# Optional:
#   --frames <int>       Number of frames to render (default: 60)
#   --preset <name>      Scene preset (default: mixed_shader)
#   --shader <name>      Shader mode (default: blinnphong)
#   --width <int>        Image width (default: 800)
#   --height <int>       Image height (default: 600)
#   --fps <int>          Output video frame rate (default: 30)
#   --aa <on|off>        Anti-aliasing (default: off)
#   --output <file>      Output video file (default: movie.mp4)
#   --renderer <path>    Path to renderer binary (default: buildVCPkg/src/raytracer)
#   -- <extra args>      Additional arguments passed through to the renderer

set -euo pipefail

# --- Defaults ---
FRAMES=60
PRESET="mixed_shader"
SHADER="blinnphong"
WIDTH=800
HEIGHT=600
FPS=30
AA="off"
OUTPUT="movie.mp4"
RENDERER="buildVCPkg/src/raytracer"
PARAM=""
MIN=""
MAX=""
EXTRA_ARGS=()

# --- Parse arguments ---
while [[ $# -gt 0 ]]; do
  case "$1" in
    --param)   PARAM="$2";   shift 2 ;;
    --min)     MIN="$2";     shift 2 ;;
    --max)     MAX="$2";     shift 2 ;;
    --frames)  FRAMES="$2";  shift 2 ;;
    --preset)  PRESET="$2";  shift 2 ;;
    --shader)  SHADER="$2";  shift 2 ;;
    --width)   WIDTH="$2";   shift 2 ;;
    --height)  HEIGHT="$2";  shift 2 ;;
    --fps)     FPS="$2";     shift 2 ;;
    --aa)      AA="$2";      shift 2 ;;
    --output)  OUTPUT="$2";  shift 2 ;;
    --renderer) RENDERER="$2"; shift 2 ;;
    --)        shift; EXTRA_ARGS+=("$@"); break ;;
    *)
      echo "Unknown option: $1" >&2
      exit 1
      ;;
  esac
done

# --- Validate required args ---
if [[ -z "$PARAM" || -z "$MIN" || -z "$MAX" ]]; then
  echo "Usage: $0 --param <name> --min <float> --max <float> [options]" >&2
  exit 1
fi

if [[ ! -x "$RENDERER" ]]; then
  echo "Error: renderer not found at '$RENDERER'" >&2
  exit 1
fi

# --- Set up temp directory ---
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "Rendering $FRAMES frames: $PARAM from $MIN to $MAX"
echo "  preset=$PRESET shader=$SHADER ${WIDTH}x${HEIGHT} aa=$AA"

# --- Render frames ---
for i in $(seq 0 $((FRAMES - 1))); do
  # Linearly interpolate: value = min + (max - min) * i / (frames - 1)
  if [[ "$FRAMES" -eq 1 ]]; then
    VALUE="$MIN"
  else
    VALUE=$(awk "BEGIN { printf \"%.6f\", $MIN + ($MAX - $MIN) * $i / ($FRAMES - 1) }")
  fi

  FRAME_FILE=$(printf "%s/frame_%04d.png" "$TMPDIR" "$i")

  printf "\r  frame %d/%d  %s=%.3f" "$((i + 1))" "$FRAMES" "$PARAM" "$VALUE"

  "$RENDERER" \
    --scene-preset "$PRESET" \
    --shader "$SHADER" \
    --width "$WIDTH" \
    --height "$HEIGHT" \
    --anti-aliasing "$AA" \
    --scene-param "${PARAM}=${VALUE}" \
    "${EXTRA_ARGS[@]+"${EXTRA_ARGS[@]}"}" \
    > "$FRAME_FILE"
done

echo ""
echo "Encoding $OUTPUT at ${FPS} fps..."

# Choose encoding based on output file extension
case "$OUTPUT" in
  *.gif)
    ffmpeg -y -framerate "$FPS" -i "$TMPDIR/frame_%04d.png" \
      -vf "split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" \
      "$OUTPUT" 2>/dev/null
    ;;
  *)
    ffmpeg -y -framerate "$FPS" -i "$TMPDIR/frame_%04d.png" \
      -c:v libx264 -pix_fmt yuv420p -movflags +faststart \
      "$OUTPUT" 2>/dev/null
    ;;
esac

echo "Done: $OUTPUT"
