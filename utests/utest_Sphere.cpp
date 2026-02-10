#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Sphere.h"
#include "Ray.h"

TEST_CASE("Sphere basic operations", "[Sphere]") {
    vec3 center(0.0f, 0.0f, 0.0f);
    float radius = 1.0f;
    vec3 material(1.0f, 0.0f, 0.0f);
    Sphere sphere(center, radius, material);

    SECTION("Get center") {
        vec3 result = sphere.getCenter();
        REQUIRE(result[0] == 0.0f);
        REQUIRE(result[1] == 0.0f);
        REQUIRE(result[2] == 0.0f);
    }

    SECTION("Get radius") {
        REQUIRE(sphere.getRadius() == 1.0f);
    }

    SECTION("Get material") {
        vec3 result = sphere.getMaterial().color;
        REQUIRE(result[0] == 1.0f);
        REQUIRE(result[1] == 0.0f);
        REQUIRE(result[2] == 0.0f);
    }
}

TEST_CASE("Sphere ray intersection - discriminant cases", "[Sphere]") {
    vec3 center(0.0f, 0.0f, 0.0f);
    float radius = 1.0f;
    vec3 material(1.0f, 1.0f, 1.0f);
    Sphere sphere(center, radius, material);

    SECTION("No intersection - ray misses sphere (negative discriminant)") {
        // Ray starting at (0, 2, 0) pointing in x direction, sphere at origin with radius 1
        vec3 origin(0.0f, 2.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(!hit.has_value());
    }

    SECTION("Tangent intersection - ray touches sphere (zero discriminant)") {
        // Ray starting at (-1, 1, 0) pointing in x direction
        // This ray should be tangent to the sphere at origin with radius 1
        // The ray passes through points like (-1+t, 1, 0)
        // Distance to origin: sqrt((t-1)^2 + 1) should equal 1
        // This happens when t=1, giving point (0, 1, 0)
        vec3 origin(-1.0f, 1.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(1.0f, 0.01f));
        
        // Tangent point should be at (0, 1, 0)
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinRel(0.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[1], Catch::Matchers::WithinRel(1.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[2], Catch::Matchers::WithinRel(0.0f, 0.01f));
    }

    SECTION("Two intersections - ray passes through sphere (positive discriminant)") {
        // Ray from (-5, 0, 0) pointing in +x direction
        vec3 origin(-5.0f, 0.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        
        // Should hit at t = 4 (first intersection at (-1, 0, 0))
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(4.0f, 0.01f));
        
        // Intersection point should be at (-1, 0, 0)
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinRel(-1.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[1], Catch::Matchers::WithinRel(0.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[2], Catch::Matchers::WithinRel(0.0f, 0.01f));
        
        // Normal should point outward from sphere
        vec3 expected_normal = (hit.value().point - center).normalized();
        REQUIRE_THAT(hit.value().normal[0], Catch::Matchers::WithinRel(expected_normal[0], 0.01f));
        REQUIRE_THAT(hit.value().normal[1], Catch::Matchers::WithinRel(expected_normal[1], 0.01f));
        REQUIRE_THAT(hit.value().normal[2], Catch::Matchers::WithinRel(expected_normal[2], 0.01f));
    }

    SECTION("Two intersections - closest intersection is chosen") {
        // Ray from (-5, 0, 0) pointing in +x direction
        // Should hit sphere at x = -1 (t=4) and x = 1 (t=6)
        // Should return the closer intersection
        vec3 origin(-5.0f, 0.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(4.0f, 0.01f));
    }
}

TEST_CASE("Sphere ray intersection - special cases", "[Sphere]") {
    vec3 center(5.0f, 5.0f, 5.0f);
    float radius = 2.0f;
    vec3 material(0.5f, 0.5f, 0.5f);
    Sphere sphere(center, radius, material);

    SECTION("Ray origin inside sphere") {
        // Ray starting at sphere center
        vec3 origin(5.0f, 5.0f, 5.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        
        // Should hit the back of the sphere at t = 2
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(2.0f, 0.01f));
    }

    SECTION("t_min constraint - intersection at t < t_min is ignored") {
        // Ray from (0, 0, 0) pointing in x direction hits sphere at origin with radius 2
        // First intersection at t=2, second at t=6
        vec3 origin(0.0f, 0.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        // Request t_min > 2 should find second intersection at t=6, but t_max < 6
        auto hit = sphere.intersect(ray, 3.0f, 5.0f);
        REQUIRE(!hit.has_value());  // Both intersections are either < 3 or > 5
    }

    SECTION("t_max constraint - intersection at t > t_max is ignored") {
        vec3 origin(2.0f, 5.0f, 5.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        // Intersection at t = 1, but t_max = 0.5
        auto hit = sphere.intersect(ray, 0.001f, 0.5f);
        REQUIRE(!hit.has_value());
    }

    SECTION("Valid intersection within t_min and t_max bounds") {
        vec3 origin(2.0f, 5.0f, 5.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        // Intersection should be at t = 1
        auto hit = sphere.intersect(ray, 0.5f, 2.0f);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().t, Catch::Matchers::WithinRel(1.0f, 0.01f));
    }
}

TEST_CASE("Sphere ray intersection - different sphere positions", "[Sphere]") {
    vec3 material(1.0f, 0.0f, 0.0f);

    SECTION("Sphere centered at origin") {
        Sphere sphere(vec3(0.0f, 0.0f, 0.0f), 1.0f, material);
        vec3 origin(-2.0f, 0.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinRel(-1.0f, 0.01f));
    }

    SECTION("Sphere at arbitrary position") {
        Sphere sphere(vec3(10.0f, -5.0f, 3.0f), 2.0f, material);
        vec3 origin(5.0f, -5.0f, 3.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        
        // Should hit at x = 8
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinRel(8.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[1], Catch::Matchers::WithinRel(-5.0f, 0.01f));
        REQUIRE_THAT(hit.value().point[2], Catch::Matchers::WithinRel(3.0f, 0.01f));
    }
}

TEST_CASE("Sphere ray intersection - different sphere radii", "[Sphere]") {
    vec3 center(0.0f, 0.0f, 0.0f);
    vec3 material(1.0f, 1.0f, 0.0f);

    SECTION("Small radius sphere") {
        Sphere sphere(center, 0.5f, material);
        vec3 origin(-2.0f, 0.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinRel(-0.5f, 0.01f));
    }

    SECTION("Large radius sphere") {
        Sphere sphere(center, 5.0f, material);
        vec3 origin(-10.0f, 0.0f, 0.0f);
        vec3 direction(1.0f, 0.0f, 0.0f);
        Ray ray(origin, direction);

        auto hit = sphere.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit.value().point[0], Catch::Matchers::WithinRel(-5.0f, 0.01f));
    }
}

TEST_CASE("Sphere hit record material", "[Sphere]") {
    vec3 center(0.0f, 0.0f, 0.0f);
    vec3 material(0.2f, 0.7f, 0.3f);
    Sphere sphere(center, 1.0f, material);

    vec3 origin(-2.0f, 0.0f, 0.0f);
    vec3 direction(1.0f, 0.0f, 0.0f);
    Ray ray(origin, direction);

    auto hit = sphere.intersect(ray);
    REQUIRE(hit.has_value());
    
    // Material should be preserved in hit record
    REQUIRE_THAT(hit.value().material.color[0], Catch::Matchers::WithinRel(0.2f, 0.01f));
    REQUIRE_THAT(hit.value().material.color[1], Catch::Matchers::WithinRel(0.7f, 0.01f));
    REQUIRE_THAT(hit.value().material.color[2], Catch::Matchers::WithinRel(0.3f, 0.01f));
}
