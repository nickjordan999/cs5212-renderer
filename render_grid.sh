#!/bin/bash
set -u
cd "$(dirname "$0")"

OUT=renderings
mkdir -p "$OUT"

BIN=./buildVCPkg/src/raytracer
COMMON=(--width 800 --height 800 --shader diffuse --caustic-extent 24 --caustic-grid 4096 --caustic-blur 1 --rays-per-pixel 64 --shadows on --reflect-depth 3 -p fresnel_random_spheres)

# Base scene params
BASE_NUMBER_SPHERES=30
BASE_WATER_SIZE=24
BASE_WATER_FREQUENCY=0.75
BASE_WATER_AMPLITUDE=0.3
BASE_CAM_BACK=16
BASE_CAM_HEIGHT=6.0
BASE_WATER_RESOLUTION=512
BASE_SPHERE_SEED=43
BASE_WATER_Z=4.0
BASE_FLOOR_Z=2.0
BASE_PHOTONS=5000000

run_variant() {
    local name="$1"
    local water_freq="$2"
    local water_amp="$3"
    local cam_h="$4"
    local photons="$5"
    local out="$OUT/${name}.png"
    if [[ -s "$out" ]]; then
        echo "[skip] $out exists"
        return
    fi
    echo "[render] $name (freq=$water_freq amp=$water_amp camh=$cam_h photons=$photons)"
    local t0=$SECONDS
    "$BIN" "${COMMON[@]}" \
        --photons "$photons" \
        --scene-param "number_spheres=$BASE_NUMBER_SPHERES" \
        --scene-param "water_size=$BASE_WATER_SIZE" \
        --scene-param "water_frequency=$water_freq" \
        --scene-param "water_amplitude=$water_amp" \
        --scene-param "cam_back=$BASE_CAM_BACK" \
        --scene-param "cam_height=$cam_h" \
        --scene-param "water_resolution=$BASE_WATER_RESOLUTION" \
        --scene-param "sphere_seed=$BASE_SPHERE_SEED" \
        --scene-param "water_z=$BASE_WATER_Z" \
        --scene-param "floor_z=$BASE_FLOOR_Z" \
        > "$out"
    local dt=$((SECONDS - t0))
    echo "[done]  $name in ${dt}s"
}

# Column 1: water_frequency
for v in 0.25 0.50 0.75 1.00 1.50 2.00; do
    run_variant "water_frequency_${v}" "$v" "$BASE_WATER_AMPLITUDE" "$BASE_CAM_HEIGHT" "$BASE_PHOTONS"
done

# Column 2: water_amplitude
for v in 0.1 0.3 0.5 0.8 1.1 1.5; do
    run_variant "water_amplitude_${v}" "$BASE_WATER_FREQUENCY" "$v" "$BASE_CAM_HEIGHT" "$BASE_PHOTONS"
done

# Column 3: cam_height
for v in 2.0 3.0 4.5 6.0 7.0 8.0; do
    run_variant "cam_height_${v}" "$BASE_WATER_FREQUENCY" "$BASE_WATER_AMPLITUDE" "$v" "$BASE_PHOTONS"
done

# Column 4: photons
for v in 10000 100000 500000 1000000 5000000 10000000; do
    run_variant "photons_${v}" "$BASE_WATER_FREQUENCY" "$BASE_WATER_AMPLITUDE" "$BASE_CAM_HEIGHT" "$v"
done

echo "All renders complete."
