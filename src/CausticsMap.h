#ifndef CAUSTICS_MAP_H
#define CAUSTICS_MAP_H

#include "vec.h"
#include <vector>
#include <mutex>

// 2D accumulation grid for forward-traced photons.
// The map sits on a plane with origin `origin` and tangent axes (u_axis, v_axis),
// covering [-u_extent, +u_extent] x [-v_extent, +v_extent] from `origin`.
// Each cell accumulates photon power (vec3) and converts to Lambertian radiance on lookup.
class CausticsMap {
public:
    CausticsMap(const vec3& origin,
                const vec3& u_axis,
                const vec3& v_axis,
                float u_extent, float v_extent,
                int u_cells, int v_cells);

    // Splat a photon's power into the cell containing world point `point`.
    // Thread-safe (internally synchronized).
    void deposit(const vec3& point, const vec3& power);

    // Sample radiance at world point `point` using a 3x3 box kernel and the
    // Lambertian conversion (radiance = flux_density / pi). Returns black for
    // points outside the recorded region.
    vec3 estimate(const vec3& point) const;

    // In-place box blur (radius cells per side). Useful for smoothing photon noise.
    void blur(int radius);

    int uCells() const { return u_cells; }
    int vCells() const { return v_cells; }

private:
    bool worldToCell(const vec3& point, int& cu, int& cv) const;

    vec3 origin;
    vec3 u_axis;
    vec3 v_axis;
    float u_extent;
    float v_extent;
    int u_cells;
    int v_cells;
    float cell_area;

    std::vector<vec3> cells;     // row-major: cells[v * u_cells + u]
    mutable std::mutex mutex;
};

#endif // CAUSTICS_MAP_H
