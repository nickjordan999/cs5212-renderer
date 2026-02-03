#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Ray.h"

TEST_CASE("Ray basic operations", "[Ray]") {
    vec3 origin(0.0f, 0.0f, 0.0f);
    vec3 direction(1.0f, 0.0f, 0.0f);
    Ray ray(origin, direction);

    SECTION("Get origin") {
        vec3 ray_origin = ray.getOrigin();
        REQUIRE(ray_origin[0] == 0.0f);
        REQUIRE(ray_origin[1] == 0.0f);
        REQUIRE(ray_origin[2] == 0.0f);
    }

    SECTION("Get direction") {
        vec3 ray_direction = ray.getDirection();
        // Direction should be normalized
        REQUIRE_THAT(ray_direction.length(), Catch::Matchers::WithinRel(1.0f, 0.001f));
        REQUIRE(ray_direction[0] == 1.0f);
        REQUIRE(ray_direction[1] == 0.0f);
        REQUIRE(ray_direction[2] == 0.0f);
    }

    SECTION("Calculate point at t=0") {
        vec3 point = ray.at(0.0f);
        REQUIRE(point[0] == 0.0f);
        REQUIRE(point[1] == 0.0f);
        REQUIRE(point[2] == 0.0f);
    }

    SECTION("Calculate point at t=5") {
        vec3 point = ray.at(5.0f);
        REQUIRE(point[0] == 5.0f);
        REQUIRE(point[1] == 0.0f);
        REQUIRE(point[2] == 0.0f);
    }

    SECTION("Calculate point at negative t") {
        vec3 point = ray.at(-3.0f);
        REQUIRE(point[0] == -3.0f);
        REQUIRE(point[1] == 0.0f);
        REQUIRE(point[2] == 0.0f);
    }
}

TEST_CASE("Ray with non-unit direction", "[Ray]") {
    vec3 origin(1.0f, 2.0f, 3.0f);
    vec3 direction(3.0f, 4.0f, 0.0f);  // Not normalized, length = 5
    Ray ray(origin, direction);

    SECTION("Direction is normalized") {
        vec3 ray_direction = ray.getDirection();
        REQUIRE_THAT(ray_direction.length(), Catch::Matchers::WithinRel(1.0f, 0.001f));
    }

    SECTION("Calculate point at t=1") {
        vec3 point = ray.at(1.0f);
        // direction normalized: (3/5, 4/5, 0)
        // point = (1, 2, 3) + 1 * (3/5, 4/5, 0) = (1.6, 2.8, 3)
        REQUIRE_THAT(point[0], Catch::Matchers::WithinRel(1.6f, 0.001f));
        REQUIRE_THAT(point[1], Catch::Matchers::WithinRel(2.8f, 0.001f));
        REQUIRE_THAT(point[2], Catch::Matchers::WithinRel(3.0f, 0.001f));
    }
}

TEST_CASE("Ray with 3D direction", "[Ray]") {
    vec3 origin(0.0f, 0.0f, 0.0f);
    vec3 direction(1.0f, 1.0f, 1.0f);
    Ray ray(origin, direction);

    SECTION("Direction is normalized") {
        vec3 ray_direction = ray.getDirection();
        REQUIRE_THAT(ray_direction.length(), Catch::Matchers::WithinRel(1.0f, 0.001f));
    }

    SECTION("Calculate point along diagonal at t=sqrt(3)") {
        float t = std::sqrt(3.0f);
        vec3 point = ray.at(t);
        // normalized direction: (1/sqrt(3), 1/sqrt(3), 1/sqrt(3))
        // point = (0, 0, 0) + sqrt(3) * (1/sqrt(3), 1/sqrt(3), 1/sqrt(3)) = (1, 1, 1)
        REQUIRE_THAT(point[0], Catch::Matchers::WithinRel(1.0f, 0.001f));
        REQUIRE_THAT(point[1], Catch::Matchers::WithinRel(1.0f, 0.001f));
        REQUIRE_THAT(point[2], Catch::Matchers::WithinRel(1.0f, 0.001f));
    }
}

TEST_CASE("Ray from arbitrary points", "[Ray]") {
    vec3 origin(5.0f, 10.0f, 15.0f);
    vec3 direction(0.0f, 1.0f, 0.0f);  // Pointing in +Y direction
    Ray ray(origin, direction);

    SECTION("Point at t=0 is the origin") {
        vec3 point = ray.at(0.0f);
        REQUIRE(point[0] == 5.0f);
        REQUIRE(point[1] == 10.0f);
        REQUIRE(point[2] == 15.0f);
    }

    SECTION("Point moves along Y axis") {
        vec3 point = ray.at(7.0f);
        REQUIRE(point[0] == 5.0f);
        REQUIRE(point[1] == 17.0f);
        REQUIRE(point[2] == 15.0f);
    }
}
