#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <string>
#include "vec.h"
#include "png++/png.hpp"

class FrameBuffer {
public:
    FrameBuffer(size_t width, size_t height);
    ~FrameBuffer() = default;

    // Access pixel at (x, y)
    vec3& operator()(size_t x, size_t y);
    const vec3& operator()(size_t x, size_t y) const;

    // Get dimensions
    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }

    // Set all pixels to a background color
    void setBackground(const vec3& color);

    void setPixel(size_t x, size_t y, const vec3 &color);

    // Write the frame buffer to a PNG file
    void writeToPng(const std::string& filename) const;

    // Get raw data (for writing to PNG later)
    const std::vector<vec3>& getData() const { return data; }

private:
    size_t width;
    size_t height;
    std::vector<vec3> data;
};

#endif // FRAMEBUFFER_H