#include "FrameBuffer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

FrameBuffer::FrameBuffer(size_t width, size_t height)
  : width(width), height(height), data(width * height)
{
}

vec3 &FrameBuffer::operator()(size_t x, size_t y)
{
  return data[y * width + x];
}

const vec3 &FrameBuffer::operator()(size_t x, size_t y) const
{
  return data[y * width + x];
}

void FrameBuffer::setBackground(const vec3 &color)
{
  std::fill(data.begin(), data.end(), color);
}

void FrameBuffer::setPixel(size_t x, size_t y, const vec3 &color)
{
  (*this)(x, y) = color;
}

void FrameBuffer::writeToPng(const std::string &filename, float gamma) const
{
  png::image<png::rgb_pixel> image(width, height);

  const float inv_gamma = (gamma > 0.0f) ? (1.0f / gamma) : 1.0f;
  const bool apply_gamma = (gamma > 0.0f) && (gamma != 1.0f);

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      const vec3 &color = (*this)(x, height - 1 - y);

      float lr = std::max(color[0], 0.0f);
      float lg = std::max(color[1], 0.0f);
      float lb = std::max(color[2], 0.0f);

      float cr = apply_gamma ? std::pow(lr, inv_gamma) : lr;
      float cg = apply_gamma ? std::pow(lg, inv_gamma) : lg;
      float cb = apply_gamma ? std::pow(lb, inv_gamma) : lb;

      png::byte r = static_cast<png::byte>(std::clamp(cr * 255.0f, 0.0f, 255.0f));
      png::byte g = static_cast<png::byte>(std::clamp(cg * 255.0f, 0.0f, 255.0f));
      png::byte b = static_cast<png::byte>(std::clamp(cb * 255.0f, 0.0f, 255.0f));

      image[y][x] = png::rgb_pixel(r, g, b);
    }
  }

  image.write(filename);
}
