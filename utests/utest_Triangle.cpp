#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Triangle.h"
#include "Ray.h"

TEST_CASE("Triangle basic operations", "[Triangle]") {
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 material(1.0f, 0.0f, 0.0f);
    Triangle tri(v0, v1, v2, material);

    SECTION("Get vertices") {
        REQUIRE(tri.getV0()[0] == 0.0f);
        REQUIRE(tri.getV0()[1] == 0.0f);
        REQUIRE(tri.getV0()[2] == 0.0f);

        REQUIRE(tri.getV1()[0] == 1.0f);
        REQUIRE(tri.getV1()[1] == 0.0f);
        REQUIRE(tri.getV1()[2] == 0.0f);

        REQUIRE(tri.getV2()[0] == 0.0f);
        REQUIRE(tri.getV2()[1] == 1.0f);
        REQUIRE(tri.getV2()[2] == 0.0f);
    }

    SECTION("Get material") {
        vec3 result = tri.getMaterial();
        REQUIRE(result[0] == 1.0f);
        REQUIRE(result[1] == 0.0f);
        REQUIRE(result[2] == 0.0f);
    }

    SECTION("Get face normal") {
        vec3 normal = tri.getNormal();
        // Normal should be normalized
        REQUIRE_THAT(normal.length(), Catch::Matchers::WithinRel(1.0f, 0.001f));
        // For triangle in XY plane (v0-v1 along x, v0-v2 along y), normal should point in +z
        REQUIRE_THAT(normal[0], Catch::Matchers::WithinAbs(0.0f, 0.01f));
        REQUIRE_THAT(normal[1], Catch::Matchers::WithinAbs(0.0f, 0.01f));
        REQUIRE_THAT(normal[2], Catch::Matchers::WithinRel(1.0f, 0.01f));
    }
}

TEST_CASE("Triangle ray intersection - front face", "[Triangle]") {
    // Create triangle in XY plane
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 material(1.0f, 1.0f, 1.0f);
    Triangle tri(v0, v1, v2, material);

    SECTION("Ray perpendicular to triangle - hits center") {
        // Ray from +z pointing toward origin
        vec3 origin(0.5f, 0.25f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        
        // Should intersect at (0.5, 0.25, 0)
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(5.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinAbs(0.5f, 0.01f));
        REQUIRE_THAT(hit.value().point[1], Catch::Matchers::WithinAbs(0.25f, 0.01f));
        REQUIRE_THAT(hit.value().point[2], Catch::Matchers::WithinAbs(0.0f, 0.01f));
        
        // Material should be returned
        REQUIRE(hit.value().material[0] == 1.0f);
    }

    SECTION("Ray perpendicular to triangle - hits vertex v0") {
        // Ray toward v0 (origin)
        vec3 origin(0.0f, 0.0f, 10.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(10.0f, 0.01f));
    }

    SECTION("Ray perpendicular to triangle - hits edge") {
        // Ray toward midpoint of edge v0-v1
        vec3 origin(0.5f, 0.0f, 10.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(10.0f, 0.01f));
    }

    SECTION("Ray misses triangle - outside bounds") {
        // Ray pointing outside the triangle
        vec3 origin(2.0f, 2.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(!hit.has_value());
    }

    SECTION("Ray parallel to triangle") {
        // Ray parallel to triangle plane
        vec3 origin(0.5f, 0.5f, 1.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(!hit.has_value());
    }
}

TEST_CASE("Triangle ray intersection - back face", "[Triangle]") {
    // Triangle in XY plane
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 material(1.0f, 1.0f, 1.0f);
    Triangle tri(v0, v1, v2, material);

    SECTION("Ray from behind (negative z) pointing through triangle") {
        // Ray from -z pointing toward origin - should hit from back
        vec3 origin(0.5f, 0.25f, -5.0f);
        vec3 direction(0.0f, 0.0f, 1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(5.0f, 0.01f));
    }
}

TEST_CASE("Triangle ray intersection - t bounds checking", "[Triangle]") {
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 material(1.0f, 1.0f, 1.0f);
    Triangle tri(v0, v1, v2, material);

    SECTION("Ray intersection within default bounds") {
        vec3 origin(0.5f, 0.25f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray, 0.001f, 1e6f);
        REQUIRE(hit.has_value());
    }

    SECTION("Ray intersection with t < t_min is rejected") {
        vec3 origin(0.5f, 0.25f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        // t = 5, but t_min = 6, so should reject
        auto hit = tri.intersect(ray, 6.0f, 1e6f);
        REQUIRE(!hit.has_value());
    }

    SECTION("Ray intersection with t > t_max is rejected") {
        vec3 origin(0.5f, 0.25f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        // t = 5, but t_max = 4, so should reject
        auto hit = tri.intersect(ray, 0.001f, 4.0f);
        REQUIRE(!hit.has_value());
    }

    SECTION("Ray intersection with t at boundary t_min") {
        vec3 origin(0.5f, 0.25f, 5.001f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        // t ≈ 5, t_min ≈ 5, should accept (small epsilon)
        auto hit = tri.intersect(ray, 4.9f, 1e6f);
        REQUIRE(hit.has_value());
    }
}

TEST_CASE("Triangle ray intersection - barycentric coordinate validation", "[Triangle]") {
    // Create a larger triangle for easier testing
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(4.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 4.0f, 0.0f);
    vec3 material(1.0f, 1.0f, 1.0f);
    Triangle tri(v0, v1, v2, material);

    SECTION("Ray hits interior of triangle") {
        // Point at (1, 1, 0) is inside triangle
        vec3 origin(1.0f, 1.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
    }

    SECTION("Ray hits near edge v0-v1") {
        // Point at (2, 0.1, 0) is inside triangle
        vec3 origin(2.0f, 0.1f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
    }

    SECTION("Ray hits near edge v0-v2") {
        // Point at (0.1, 2, 0) is inside triangle
        vec3 origin(0.1f, 2.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
    }

    SECTION("Ray hits near edge v1-v2") {
        // Point at (1, 1.5, 0) - need to check if it's inside
        // Triangle vertices: (0,0), (4,0), (0,4)
        // Line from v1 to v2: from (4,0) to (0,4)
        // Equation: x + y = 4
        // Point (1, 1.5): 1 + 1.5 = 2.5 < 4, so inside
        vec3 origin(1.0f, 1.5f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
    }

    SECTION("Ray misses - u out of bounds (negative)") {
        // Point at (-1, 1, 0) is outside
        vec3 origin(-1.0f, 1.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(!hit.has_value());
    }

    SECTION("Ray misses - u out of bounds (> 1)") {
        // Point at (5, 0, 0) is outside (x > 4)
        vec3 origin(5.0f, 0.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(!hit.has_value());
    }

    SECTION("Ray misses - v out of bounds (negative)") {
        // Point at (1, -1, 0) is outside
        vec3 origin(1.0f, -1.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(!hit.has_value());
    }

    SECTION("Ray misses - u + v > 1") {
        // Point at (2.5, 2.5, 0): u+v = 1.25 + 0.625 > 1, outside
        vec3 origin(2.5f, 2.5f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(!hit.has_value());
    }
}

TEST_CASE("Triangle ray intersection - angled ray", "[Triangle]") {
    // Triangle in XY plane
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(2.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 2.0f, 0.0f);
    vec3 material(0.5f, 0.5f, 0.5f);
    Triangle tri(v0, v1, v2, material);

    SECTION("Diagonal ray through triangle") {
        // Ray from above, pointing down and toward triangle
        // Start at (1, 1, 5) pointing toward (1, 1, 0)
        vec3 origin(1.0f, 1.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        
        // Ray hits at t=5, position (1, 1, 0)
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(5.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinAbs(1.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[1], Catch::Matchers::WithinAbs(1.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[2], Catch::Matchers::WithinAbs(0.0f, 0.01f));
    }
}

TEST_CASE("Triangle ray intersection - HitRecord content", "[Triangle]") {
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 material(0.2f, 0.8f, 0.4f);
    Triangle tri(v0, v1, v2, material);

    vec3 origin(0.33f, 0.33f, 5.0f);
    vec3 direction(0.0f, 0.0f, -1.0f);
    Ray ray(origin, direction);

    auto hit = tri.intersect(ray);
    REQUIRE(hit.has_value());

    SECTION("HitRecord has correct t value") {
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(5.0f, 0.01f));
    }

    SECTION("HitRecord has correct point") {
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinAbs(0.33f, 0.01f));
        REQUIRE_THAT(hit.value().point[1], Catch::Matchers::WithinAbs(0.33f, 0.01f));
        REQUIRE_THAT(hit.value().point[2], Catch::Matchers::WithinAbs(0.0f, 0.01f));
    }

    SECTION("HitRecord has correct material") {
        REQUIRE_THAT(hit.value().material[0], Catch::Matchers::WithinAbs(0.2f, 0.001f));
        REQUIRE_THAT(hit.value().material[1], Catch::Matchers::WithinAbs(0.8f, 0.001f));
        REQUIRE_THAT(hit.value().material[2], Catch::Matchers::WithinAbs(0.4f, 0.001f));
    }

    SECTION("HitRecord has normalized normal") {
        vec3 normal = hit.value().normal;
        REQUIRE_THAT(normal.length(), Catch::Matchers::WithinRel(1.0f, 0.001f));
    }
}

TEST_CASE("Triangle with different vertex orders", "[Triangle]") {
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    vec3 material(1.0f, 1.0f, 1.0f);

    SECTION("Standard winding order") {
        Triangle tri(v0, v1, v2, material);
        vec3 origin(0.25f, 0.25f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
    }

    SECTION("Reversed winding order (different face direction)") {
        Triangle tri(v0, v2, v1, material);  // Reverse v1 and v2
        vec3 origin(0.25f, 0.25f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        // Should still intersect, but normal will be opposite
        REQUIRE(hit.has_value());
        
        // Normal should point in opposite direction (-z instead of +z)
        REQUIRE_THAT(hit.value().normal[2], Catch::Matchers::WithinRel(-1.0f, 0.01f));
    }
}

TEST_CASE("Triangle degenerate cases", "[Triangle]") {
    SECTION("Triangle with very small area") {
        // Even with small area, intersection should still work if ray hits it
        vec3 v0(0.0f, 0.0f, 0.0f);
        vec3 v1(0.001f, 0.0f, 0.0f);
        vec3 v2(0.0f, 0.001f, 0.0f);
        vec3 material(1.0f, 1.0f, 1.0f);
        Triangle tri(v0, v1, v2, material);

        // Ray aimed directly at center of small triangle
        vec3 origin(0.0f, 0.0f, 5.0f);
        vec3 direction(0.0f, 0.0f, -1.0f);
        Ray ray(origin, direction);

        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
    }
}
