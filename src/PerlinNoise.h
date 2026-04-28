#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <array>
#include <numeric>
#include <random>
#include <cmath>

// Classic Ken Perlin gradient noise (2D). Constructs a seeded permutation of
// 0..255 doubled to length 512 for wrap-around indexing. noise2D returns values
// in roughly [-1, 1].
class PerlinNoise {
public:
    explicit PerlinNoise(uint32_t seed = 42) {
        std::array<int, 256> base;
        std::iota(base.begin(), base.end(), 0);
        std::mt19937 rng(seed);
        std::shuffle(base.begin(), base.end(), rng);
        for (int i = 0; i < 512; ++i) {
            p[i] = base[i & 255];
        }
    }

    float noise2D(float x, float y) const {
        int xi = static_cast<int>(std::floor(x)) & 255;
        int yi = static_cast<int>(std::floor(y)) & 255;
        float xf = x - std::floor(x);
        float yf = y - std::floor(y);
        float u = fade(xf);
        float v = fade(yf);

        int aa = p[p[xi]     + yi];
        int ab = p[p[xi]     + yi + 1];
        int ba = p[p[xi + 1] + yi];
        int bb = p[p[xi + 1] + yi + 1];

        float x1 = lerp(grad(aa, xf,        yf       ),
                        grad(ba, xf - 1.0f, yf       ), u);
        float x2 = lerp(grad(ab, xf,        yf - 1.0f),
                        grad(bb, xf - 1.0f, yf - 1.0f), u);

        return lerp(x1, x2, v);
    }

private:
    std::array<int, 512> p;

    static float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }
    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
    static float grad(int hash, float x, float y) {
        // 8 gradient directions on the unit circle, picked from the low 3 bits of hash.
        switch (hash & 7) {
            case 0: return  x + y;
            case 1: return -x + y;
            case 2: return  x - y;
            case 3: return -x - y;
            case 4: return  x;
            case 5: return -x;
            case 6: return  y;
            default: return -y;
        }
    }
};

#endif // PERLIN_NOISE_H
