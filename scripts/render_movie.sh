#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat >&2 <<EOF
Usage: $(basename "$0") [OPTIONS]

Render a sequence of frames by sweeping a scene parameter, then encode to video.

Required:
  --t-min <float>       Starting value of t
  --t-max <float>       Ending value of t
  --num-steps <int>     Number of frames to render

Optional:
  --preset <name>       Scene preset (default: spiral)
  --shader <name>       Shader mode (default: diffuse)
  --width <int>         Image width (default: 800)
  --height <int>        Image height (default: 800)
  --fps <int>           Video frame rate (default: 30)
  --rays-per-pixel <n>  Rays per pixel / AA samples (default: 8)
  --anti-aliasing <v>   on or off (default: on)
  --output <path>       Output video file (default: output.mp4)
  --keep-frames         Keep the frame directory after encoding
  --raytracer <path>    Path to raytracer binary
                        (default: build/src/raytracer relative to repo root)

Example:
  $(basename "$0") --t-min 0 --t-max 0.95 --num-steps 120 --width 400 --height 400
EOF
  exit 1
}

# ── Defaults ──────────────────────────────────────────────────────────────────
preset="spiral"
shader="diffuse"
width=800
height=800
fps=30
rays_per_pixel=8
anti_aliasing="on"
output="output.mp4"
keep_frames=false
t_min=""
t_max=""
num_steps=""

# Locate repo root (script lives in <repo>/scripts/)
repo_root="$(cd "$(dirname "$0")/.." && pwd)"
raytracer="${repo_root}/build/src/raytracer"

# ── Parse arguments ───────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
  case "$1" in
    --t-min)        t_min="$2";          shift 2 ;;
    --t-max)        t_max="$2";          shift 2 ;;
    --num-steps)    num_steps="$2";       shift 2 ;;
    --preset)       preset="$2";          shift 2 ;;
    --shader)       shader="$2";          shift 2 ;;
    --width)        width="$2";           shift 2 ;;
    --height)       height="$2";          shift 2 ;;
    --fps)          fps="$2";             shift 2 ;;
    --rays-per-pixel) rays_per_pixel="$2"; shift 2 ;;
    --anti-aliasing)  anti_aliasing="$2";  shift 2 ;;
    --output)       output="$2";          shift 2 ;;
    --keep-frames)  keep_frames=true;     shift ;;
    --raytracer)    raytracer="$2";       shift 2 ;;
    -h|--help)      usage ;;
    *)              echo "Unknown option: $1" >&2; usage ;;
  esac
done

# ── Validate required args ────────────────────────────────────────────────────
if [[ -z "$t_min" || -z "$t_max" || -z "$num_steps" ]]; then
  echo "Error: --t-min, --t-max, and --num-steps are required." >&2
  usage
fi

if ! [[ "$num_steps" =~ ^[0-9]+$ ]] || [[ "$num_steps" -lt 1 ]]; then
  echo "Error: --num-steps must be a positive integer." >&2
  exit 1
fi

if [[ ! -x "$raytracer" ]]; then
  echo "Error: raytracer not found at ${raytracer}" >&2
  echo "       Build first with: make -C build" >&2
  exit 1
fi

# ── Set up working directory ──────────────────────────────────────────────────
frame_dir=$(mktemp -d "${TMPDIR:-/tmp}/render_movie_XXXXXX")
echo "Frames directory: ${frame_dir}"

cleanup() {
  if [[ "$keep_frames" == false ]]; then
    rm -rf "$frame_dir"
  fi
}
trap cleanup EXIT

# ── Render frames ─────────────────────────────────────────────────────────────
echo "Rendering ${num_steps} frames  t=[${t_min}, ${t_max}]  ${width}x${height}"

for (( i = 0; i < num_steps; i++ )); do
  # Compute t for this frame
  if [[ "$num_steps" -eq 1 ]]; then
    t="$t_min"
  else
    t=$(awk "BEGIN {printf \"%.6f\", ${t_min} + (${t_max} - ${t_min}) * ${i} / (${num_steps} - 1)}")
  fi

  frame_file=$(printf "%s/frame_%05d.png" "$frame_dir" "$i")

  printf "\r  Frame %d/%d  t=%s" $((i + 1)) "$num_steps" "$t"

  "$raytracer" \
    -p "$preset" \
    -s "$shader" \
    -w "$width" \
    -h "$height" \
    --rays-per-pixel "$rays_per_pixel" \
    --anti-aliasing "$anti_aliasing" \
    --scene-param "t=${t}" \
    > "$frame_file"
done

echo ""
echo "Rendering complete."

# ── Encode video ──────────────────────────────────────────────────────────────
echo "Encoding video: ${output}"

ffmpeg -y -framerate "$fps" \
  -i "${frame_dir}/frame_%05d.png" \
  -c:v libx264 -pix_fmt yuv420p \
  -movflags +faststart \
  "$output" \
  2>/dev/null

echo "Done: ${output}"

if [[ "$keep_frames" == true ]]; then
  echo "Frames kept at: ${frame_dir}"
fi
