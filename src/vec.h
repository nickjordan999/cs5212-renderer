#ifndef VEC_H
#define VEC_H

#include <array>
#include <cmath>
#include <type_traits>

template <typename T, size_t N>
class vec {
public:
    std::array<T, N> data;

    // Default constructor
    vec() = default;

    // Constructor from array
    vec(const std::array<T, N>& arr) : data(arr) {}

    // Variadic constructor
    template <typename... Args>
    vec(Args... args) : data{static_cast<T>(args)...} {
        static_assert(sizeof...(Args) == N, "Number of arguments must match vector dimension");
    }

    // Access operators
    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }

    // Vector addition
    vec operator+(const vec& other) const {
        vec result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = data[i] + other[i];
        }
        return result;
    }

    // Vector subtraction
    vec operator-(const vec& other) const {
        vec result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = data[i] - other[i];
        }
        return result;
    }

    // Scalar multiplication
    vec operator*(T scalar) const {
        vec result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = data[i] * scalar;
        }
        return result;
    }

    // Scalar division
    vec operator/(T scalar) const {
        vec result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = data[i] / scalar;
        }
        return result;
    }

    // Compound assignment operators
    vec& operator+=(const vec& other) {
        for (size_t i = 0; i < N; ++i) {
            data[i] += other[i];
        }
        return *this;
    }

    vec& operator-=(const vec& other) {
        for (size_t i = 0; i < N; ++i) {
            data[i] -= other[i];
        }
        return *this;
    }

    vec& operator*=(T scalar) {
        for (size_t i = 0; i < N; ++i) {
            data[i] *= scalar;
        }
        return *this;
    }

    vec& operator/=(T scalar) {
        for (size_t i = 0; i < N; ++i) {
            data[i] /= scalar;
        }
        return *this;
    }

    // Length (magnitude)
    T length() const {
        T sum = 0;
        for (const auto& val : data) {
            sum += val * val;
        }
        return std::sqrt(sum);
    }

    // Squared length (useful for comparisons)
    T length_squared() const {
        T sum = 0;
        for (const auto& val : data) {
            sum += val * val;
        }
        return sum;
    }

    // Dot product
    T dot(const vec& other) const {
        T sum = 0;
        for (size_t i = 0; i < N; ++i) {
            sum += data[i] * other[i];
        }
        return sum;
    }

    // Cross product (only for 3D vectors)
    vec cross(const vec& other) const {
        static_assert(N == 3, "Cross product is only defined for 3D vectors");
        return vec(
            data[1] * other[2] - data[2] * other[1],
            data[2] * other[0] - data[0] * other[2],
            data[0] * other[1] - data[1] * other[0]
        );
    }

    // Check if two vectors are within epsilon distance
    bool near(const vec& other, T epsilon = T(1e-5)) const {
        vec diff = other - *this;
        return diff.length_squared() <= epsilon * epsilon;
    }

    // Return a normalized (unit) vector
    vec normalized() const {
        T len = length();
        if (len == T(0)) {
            return vec();  // Return zero vector if length is zero
        }
        return *this / len;
    }
};

// Scalar multiplication (commutative)
template <typename T, size_t N>
vec<T, N> operator*(T scalar, const vec<T, N>& v) {
    return v * scalar;
}

// Type aliases
using vec3f = vec<float, 3>;
using vec3d = vec<double, 3>;
using vec2f = vec<float, 2>;
using vec2d = vec<double, 2>;

// For convenience, vec3 as vec3f
using vec3 = vec3f;

#endif // VEC_H