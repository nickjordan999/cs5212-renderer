#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "vec.h"

TEST_CASE("vec3 basic operations", "[vec3]") {
    vec3 v1(1.0f, 2.0f, 3.0f);
    vec3 v2(4.0f, 5.0f, 6.0f);

    SECTION("Access elements") {
        REQUIRE(v1[0] == 1.0f);
        REQUIRE(v1[1] == 2.0f);
        REQUIRE(v1[2] == 3.0f);
    }

    SECTION("Addition") {
        vec3 result = v1 + v2;
        REQUIRE(result[0] == 5.0f);
        REQUIRE(result[1] == 7.0f);
        REQUIRE(result[2] == 9.0f);
    }

    SECTION("Subtraction") {
        vec3 result = v2 - v1;
        REQUIRE(result[0] == 3.0f);
        REQUIRE(result[1] == 3.0f);
        REQUIRE(result[2] == 3.0f);
    }

    SECTION("Scalar multiplication") {
        vec3 result = v1 * 2.0f;
        REQUIRE(result[0] == 2.0f);
        REQUIRE(result[1] == 4.0f);
        REQUIRE(result[2] == 6.0f);

        vec3 result2 = 3.0f * v1;
        REQUIRE(result2[0] == 3.0f);
        REQUIRE(result2[1] == 6.0f);
        REQUIRE(result2[2] == 9.0f);
    }

    SECTION("Length") {
        vec3 v(3.0f, 4.0f, 0.0f);
        REQUIRE_THAT(v.length(), Catch::Matchers::WithinRel(5.0f, 0.001f));
    }
}