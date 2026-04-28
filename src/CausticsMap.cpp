#include "CausticsMap.h"
#include <cmath>
#include <algorithm>

CausticsMap::CausticsMap(const vec3& origin,
                         const vec3& u_axis,
                         const vec3& v_axis,
                         float u_extent, float v_extent,
                         int u_cells, int v_cells)
    : origin(origin),
      u_axis(u_axis.normalized()),
      v_axis(v_axis.normalized()),
      u_extent(u_extent),
      v_extent(v_extent),
      u_cells(u_cells),
      v_cells(v_cells),
      cells(static_cast<size_t>(u_cells) * static_cast<size_t>(v_cells), vec3(0.0f, 0.0f, 0.0f))
{
    float du = (2.0f * u_extent) / static_cast<float>(u_cells);
    float dv = (2.0f * v_extent) / static_cast<float>(v_cells);
    cell_area = du * dv;
}

bool CausticsMap::worldToCell(const vec3& point, int& cu, int& cv) const {
    vec3 rel = point - origin;
    float u = rel.dot(u_axis);
    float v = rel.dot(v_axis);
    if (u < -u_extent || u >= u_extent) return false;
    if (v < -v_extent || v >= v_extent) return false;
    cu = static_cast<int>(std::floor(((u + u_extent) / (2.0f * u_extent)) * u_cells));
    cv = static_cast<int>(std::floor(((v + v_extent) / (2.0f * v_extent)) * v_cells));
    if (cu < 0) cu = 0;
    if (cu >= u_cells) cu = u_cells - 1;
    if (cv < 0) cv = 0;
    if (cv >= v_cells) cv = v_cells - 1;
    return true;
}

void CausticsMap::deposit(const vec3& point, const vec3& power) {
    int cu, cv;
    if (!worldToCell(point, cu, cv)) return;
    std::lock_guard<std::mutex> lock(mutex);
    cells[static_cast<size_t>(cv) * u_cells + cu] += power;
}

vec3 CausticsMap::estimate(const vec3& point) const {
    int cu, cv;
    if (!worldToCell(point, cu, cv)) return vec3(0.0f, 0.0f, 0.0f);

    // 3x3 box kernel — averages contributions from a 3-cell neighborhood and
    // divides by the kernel area to produce flux density.
    vec3 accum(0.0f, 0.0f, 0.0f);
    int kernel = 0;
    for (int dv = -1; dv <= 1; ++dv) {
        for (int du = -1; du <= 1; ++du) {
            int u = cu + du;
            int v = cv + dv;
            if (u < 0 || u >= u_cells) continue;
            if (v < 0 || v >= v_cells) continue;
            accum += cells[static_cast<size_t>(v) * u_cells + u];
            ++kernel;
        }
    }
    if (kernel == 0) return vec3(0.0f, 0.0f, 0.0f);
    float area = cell_area * static_cast<float>(kernel);
    static const float INV_PI = 1.0f / 3.14159265358979323846f;
    return accum * (INV_PI / area);
}

void CausticsMap::blur(int radius) {
    if (radius <= 0) return;
    std::vector<vec3> tmp(cells.size(), vec3(0.0f, 0.0f, 0.0f));

    // Horizontal pass
    for (int v = 0; v < v_cells; ++v) {
        for (int u = 0; u < u_cells; ++u) {
            vec3 sum(0.0f, 0.0f, 0.0f);
            int n = 0;
            for (int k = -radius; k <= radius; ++k) {
                int uk = u + k;
                if (uk < 0 || uk >= u_cells) continue;
                sum += cells[static_cast<size_t>(v) * u_cells + uk];
                ++n;
            }
            tmp[static_cast<size_t>(v) * u_cells + u] = sum / static_cast<float>(n);
        }
    }
    // Vertical pass
    for (int v = 0; v < v_cells; ++v) {
        for (int u = 0; u < u_cells; ++u) {
            vec3 sum(0.0f, 0.0f, 0.0f);
            int n = 0;
            for (int k = -radius; k <= radius; ++k) {
                int vk = v + k;
                if (vk < 0 || vk >= v_cells) continue;
                sum += tmp[static_cast<size_t>(vk) * u_cells + u];
                ++n;
            }
            cells[static_cast<size_t>(v) * u_cells + u] = sum / static_cast<float>(n);
        }
    }
}
