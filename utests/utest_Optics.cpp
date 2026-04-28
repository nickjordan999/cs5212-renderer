#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Optics.h"
#include "Triangle.h"
#include "Ray.h"
#include <cmath>

TEST_CASE("refract: normal incidence passes straight through", "[Optics]") {
    // Ray straight down, surface normal up (oriented against ray).
    vec3 v(0.0f, 0.0f, -1.0f);
    vec3 n(0.0f, 0.0f, 1.0f);
    auto out = refract(v, n, 1.0f / 1.33f);
    REQUIRE(out.has_value());
    REQUIRE_THAT((*out)[0], Catch::Matchers::WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT((*out)[1], Catch::Matchers::WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT((*out)[2], Catch::Matchers::WithinRel(-1.0f, 1e-5f));
}

TEST_CASE("refract: total internal reflection past critical angle", "[Optics]") {
    // Going from water (1.33) to air (1.0). Critical angle ≈ 48.75°.
    // A ray traveling at ~60° from the normal *inside* the water must TIR.
    // Set up: surface normal points back into water (against the ray).
    float angle_deg = 60.0f;
    float a = angle_deg * static_cast<float>(M_PI) / 180.0f;
    vec3 v(std::sin(a), 0.0f, -std::cos(a));  // inside water, heading down/sideways
    vec3 n(0.0f, 0.0f, 1.0f);
    // eta = n_from / n_to = 1.33 / 1.0 (going from water to air)
    auto out = refract(v, n, 1.33f / 1.0f);
    REQUIRE_FALSE(out.has_value());
}

TEST_CASE("refract: bends toward normal entering denser medium", "[Optics]") {
    // 45° incidence going from air (1.0) into water (1.33).
    float a = 45.0f * static_cast<float>(M_PI) / 180.0f;
    vec3 v(std::sin(a), 0.0f, -std::cos(a));
    vec3 n(0.0f, 0.0f, 1.0f);
    auto out = refract(v, n, 1.0f / 1.33f);
    REQUIRE(out.has_value());

    // Snell: sin(theta_t) = sin(theta_i) * (n_i / n_t) = sin(45°) / 1.33 ≈ 0.5316
    // theta_t ≈ 32.1°. The transmitted ray's x-component should equal sin(theta_t).
    float expected_sin_t = std::sin(a) / 1.33f;
    REQUIRE_THAT((*out)[0], Catch::Matchers::WithinAbs(expected_sin_t, 1e-4f));
    REQUIRE_THAT((*out)[2], Catch::Matchers::WithinAbs(-std::sqrt(1.0f - expected_sin_t * expected_sin_t), 1e-4f));
}

TEST_CASE("fresnelSchlick: matched IORs at normal incidence reflect zero", "[Optics]") {
    // r0 = ((n1 - n2)/(n1 + n2))^2 = 0 when n1 == n2; at normal incidence the
    // (1 - cos)^5 boost vanishes, so Schlick yields exactly 0.
    REQUIRE_THAT(fresnelSchlick(1.0f, 1.5f, 1.5f), Catch::Matchers::WithinAbs(0.0f, 1e-6f));
}

TEST_CASE("fresnelSchlick: normal incidence matches Schlick base reflectance", "[Optics]") {
    // Air -> water: r0 = ((1.0 - 1.33) / (1.0 + 1.33))^2 ≈ 0.02006
    float kr = fresnelSchlick(1.0f, 1.0f, 1.33f);
    REQUIRE_THAT(kr, Catch::Matchers::WithinAbs(0.02006f, 1e-3f));
}

TEST_CASE("fresnelSchlick: approaches 1 at grazing angles", "[Optics]") {
    float kr = fresnelSchlick(0.01f, 1.0f, 1.5f);
    REQUIRE(kr > 0.85f);
    REQUIRE(kr <= 1.0f);
}

TEST_CASE("Triangle: vertex-normal interpolation matches barycentric weighting", "[Triangle]") {
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);

    SECTION("All three vertex normals identical -> interpolated normal matches") {
        vec3 n_all(0.0f, 0.0f, 1.0f);
        Triangle tri(v0, v1, v2, n_all, n_all, n_all, vec3(1.0f, 1.0f, 1.0f));

        Ray ray(vec3(0.25f, 0.25f, 5.0f), vec3(0.0f, 0.0f, -1.0f));
        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit->normal[2], Catch::Matchers::WithinRel(1.0f, 1e-3f));
    }

    SECTION("Distinct vertex normals interpolate via barycentric u, v") {
        // Möller-Trumbore: u is the barycentric weight for v1, v relates to v2.
        // w = 1 - u - v is the weight for v0.
        // At hit point (0, 0, 0) the weight should be all on v0 -> n0.
        vec3 n0(1.0f, 0.0f, 0.0f);
        vec3 n1(0.0f, 1.0f, 0.0f);
        vec3 n2(0.0f, 0.0f, 1.0f);
        Triangle tri(v0, v1, v2, n0, n1, n2, vec3(1.0f, 1.0f, 1.0f));

        Ray ray_at_v0(vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, -1.0f));
        auto h0 = tri.intersect(ray_at_v0);
        REQUIRE(h0.has_value());
        // Normal should be ~(1,0,0) (n0)
        REQUIRE_THAT(h0->normal[0], Catch::Matchers::WithinAbs(1.0f, 1e-3f));
        REQUIRE_THAT(h0->normal[1], Catch::Matchers::WithinAbs(0.0f, 1e-3f));

        // At v1 the weight should be all on n1 = (0,1,0).
        Ray ray_at_v1(vec3(1.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, -1.0f));
        auto h1 = tri.intersect(ray_at_v1);
        REQUIRE(h1.has_value());
        REQUIRE_THAT(h1->normal[1], Catch::Matchers::WithinAbs(1.0f, 1e-3f));

        // At v2 the weight should be all on n2 = (0,0,1).
        Ray ray_at_v2(vec3(0.0f, 1.0f, 5.0f), vec3(0.0f, 0.0f, -1.0f));
        auto h2 = tri.intersect(ray_at_v2);
        REQUIRE(h2.has_value());
        REQUIRE_THAT(h2->normal[2], Catch::Matchers::WithinAbs(1.0f, 1e-3f));
    }
}

TEST_CASE("HitRecord: is_front_face is set", "[HitRecord]") {
    vec3 v0(0.0f, 0.0f, 0.0f);
    vec3 v1(1.0f, 0.0f, 0.0f);
    vec3 v2(0.0f, 1.0f, 0.0f);
    Triangle tri(v0, v1, v2, vec3(1.0f, 1.0f, 1.0f));

    SECTION("Front face hit") {
        Ray ray(vec3(0.25f, 0.25f, 5.0f), vec3(0.0f, 0.0f, -1.0f));
        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE(hit->is_front_face == true);
    }

    SECTION("Back face hit") {
        Ray ray(vec3(0.25f, 0.25f, -5.0f), vec3(0.0f, 0.0f, 1.0f));
        auto hit = tri.intersect(ray);
        REQUIRE(hit.has_value());
        REQUIRE(hit->is_front_face == false);
    }
}
