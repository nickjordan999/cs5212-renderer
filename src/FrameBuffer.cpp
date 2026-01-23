#include "FrameBuffer.h"
#include <algorithm>
#include <cstdint>

FrameBuffer::FrameBuffer(size_t width, size_t height)
    : width(width), height(height), data(width * height) {
}

vec3& FrameBuffer::operator()(size_t x, size_t y) {
    return data[y * width + x];
}

const vec3& FrameBuffer::operator()(size_t x, size_t y) const {
    return data[y * width + x];
}

void FrameBuffer::setBackground(const vec3& color) {
    std::fill(data.begin(), data.end(), color);
}

void FrameBuffer::setPixel(size_t x, size_t y, const vec3& color) {
    (*this)(x, y) = color;
}

void FrameBuffer::writeToPng(const std::string& filename) const {
    png::image<png::rgb_pixel> image(width, height);
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            const vec3& color = (*this)(x, y);
            
            // Convert from float [0,1] to byte [0,255]
            png::byte r = static_cast<png::byte>(std::clamp(color[0] * 255.0f, 0.0f, 255.0f));
            png::byte g = static_cast<png::byte>(std::clamp(color[1] * 255.0f, 0.0f, 255.0f));
            png::byte b = static_cast<png::byte>(std::clamp(color[2] * 255.0f, 0.0f, 255.0f));
            
            image[y][x] = png::rgb_pixel(r, g, b);
        }
    }
    
    image.write(filename);
}