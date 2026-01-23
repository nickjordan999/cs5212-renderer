#include <catch2/catch_test_macros.hpp>
#include "FrameBuffer.h"

TEST_CASE("FrameBuffer basic operations", "[FrameBuffer]") {
    FrameBuffer fb(10, 20);

    SECTION("Initialization") {
        REQUIRE(fb.getWidth() == 10);
        REQUIRE(fb.getHeight() == 20);
        REQUIRE(fb.getData().size() == 200); // 10*20
    }

    SECTION("Access pixels") {
        vec3 color(1.0f, 0.5f, 0.0f);
        fb(5, 10) = color;
        REQUIRE(fb(5, 10)[0] == 1.0f);
        REQUIRE(fb(5, 10)[1] == 0.5f);
        REQUIRE(fb(5, 10)[2] == 0.0f);
    }

    SECTION("Bounds checking") {
        // Note: In a real implementation, you might want to add bounds checking
        // For now, we assume correct usage
        REQUIRE_NOTHROW(fb(0, 0));
        REQUIRE_NOTHROW(fb(9, 19));
    }

    SECTION("Set Pixel") {
        vec3 color(0.1f, 0.2f, 0.3f);
        fb.setPixel(2, 3, color);
        REQUIRE(fb(2, 3) == color);
    }

    SECTION("Set background") {
        vec3 bgColor(0.2f, 0.8f, 0.5f);
        fb.setBackground(bgColor);
        
        // Check a few pixels to ensure they are set to the background color
        REQUIRE(fb(0, 0) == bgColor);
        REQUIRE(fb(5, 10) == bgColor);
        REQUIRE(fb(9, 19) == bgColor);
        
        // Check that all pixels in the data vector are set
        const auto& data = fb.getData();
        for (const auto& pixel : data) {
            REQUIRE(pixel == bgColor);
        }
    }

    SECTION("Write to PNG") {
        vec3 bgColor(1.0f, 0.0f, 0.0f); // Red background
        fb.setBackground(bgColor);
        
        // Test that writing to PNG doesn't throw an exception
        REQUIRE_NOTHROW(fb.writeToPng("test_output.png"));
    }
}