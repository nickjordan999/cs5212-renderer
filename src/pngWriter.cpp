#include "FrameBuffer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <cmath>
#include <unistd.h>

// Parse hex color string (format: "RRGGBB" or "#RRGGBB") to vec3
vec3 parseHexColor(const std::string& hexStr) {
    std::string hex = hexStr;
    
    // Remove '#' if present
    if (hex[0] == '#') {
        hex = hex.substr(1);
    }
    
    // Check length
    if (hex.length() != 6) {
        throw std::invalid_argument("Hex color must be in format RRGGBB or #RRGGBB");
    }
    
    // Parse hex values
    unsigned int rHex = std::stoul(hex.substr(0, 2), nullptr, 16);
    unsigned int gHex = std::stoul(hex.substr(2, 2), nullptr, 16);
    unsigned int bHex = std::stoul(hex.substr(4, 2), nullptr, 16);
    
    // Convert to float [0, 1]
    float r = rHex / 255.0f;
    float g = gHex / 255.0f;
    float b = bHex / 255.0f;
    
    return vec3(r, g, b);
}

// Structure to hold a point with color
struct ColorPoint {
    float x, y;
    vec3 color;
};

// Parse a point specification in format "x:y:RRGGBB"
ColorPoint parseColorPoint(const std::string& spec) {
    size_t pos1 = spec.find(':');
    size_t pos2 = spec.find(':', pos1 + 1);
    
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        throw std::invalid_argument("Point format must be 'x:y:RRGGBB' (e.g., '50:50:FF0000')");
    }
    
    ColorPoint point;
    point.x = std::stof(spec.substr(0, pos1));
    point.y = std::stof(spec.substr(pos1 + 1, pos2 - pos1 - 1));
    point.color = parseHexColor(spec.substr(pos2 + 1));
    
    return point;
}

void printUsage(const char* programName) {
    std::cerr << "Usage:" << std::endl;
    std::cerr << "  Solid color mode:" << std::endl;
    std::cerr << "    " << programName << " solid <width> <height> <hex_color>" << std::endl;
    std::cerr << "    width:     Image width in pixels" << std::endl;
    std::cerr << "    height:    Image height in pixels" << std::endl;
    std::cerr << "    hex_color: RGB color in hex format (e.g., FF0000 or #FF0000 for red)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Gradient mode (2-point):" << std::endl;
    std::cerr << "    " << programName << " gradient <width> <height> <start_color> <end_color> <degrees>" << std::endl;
    std::cerr << "    start_color: Starting RGB color in hex format" << std::endl;
    std::cerr << "    end_color:   Ending RGB color in hex format" << std::endl;
    std::cerr << "    degrees:     Rotation angle (0=left-to-right, 90=top-to-bottom, etc.)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Multipoint gradient mode:" << std::endl;
    std::cerr << "    " << programName << " multipoint <width> <height> <point1> <point2> [<point3> ...]" << std::endl;
    std::cerr << "    Each point format: x:y:RRGGBB (e.g., 50:50:FF0000 for red at position 50,50)" << std::endl;
    std::cerr << "    Colors interpolated using inverse distance weighting" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        // Check if gradient mode
        int width = std::stoi(argv[2]);
        int height = std::stoi(argv[3]);
        FrameBuffer fb(width, height);
            
        if (std::string(argv[1]) == "gradient") {
            if (argc != 7) {
                printUsage(argv[0]);
                return 1;
            }
            
            // Gradient mode

            std::string startColorStr = argv[4];
            std::string endColorStr = argv[5];
            float degrees = std::stof(argv[6]);
            
            // Validate dimensions
            if (width <= 0 || height <= 0) {
                std::cerr << "Error: width and height must be positive integers" << std::endl;
                return 1;
            }
            
            // Parse colors
            vec3 startColor = parseHexColor(startColorStr);
            vec3 endColor = parseHexColor(endColorStr);
            
            // Convert degrees to radians
            float radians = degrees * M_PI / 180.0f;
            float cosAngle = std::cos(radians);
            float sinAngle = std::sin(radians);
            
            // Create PNG image
            png::image<png::rgb_pixel> image(width, height);
            
            // Find the maximum distance along the gradient direction
            // This ensures the gradient spans the full image
            float maxDist = 0.0f;
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    float px = x - width / 2.0f;
                    float py = y - height / 2.0f;
                    float dist = px * cosAngle + py * sinAngle;
                    if (std::abs(dist) > maxDist) {
                        maxDist = std::abs(dist);
                    }
                }
            }
            
            if (maxDist == 0.0f) maxDist = 1.0f;
            
            // Generate gradient
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    float px = x - width / 2.0f;
                    float py = y - height / 2.0f;
                    float dist = px * cosAngle + py * sinAngle;
                    
                    // Normalize to [0, 1] range
                    float t = (dist + maxDist) / (2.0f * maxDist);
                    t = std::clamp(t, 0.0f, 1.0f);
                    
                    // Interpolate colors
                    vec3 pixelColor = startColor * (1.0f - t) + endColor * t;
                    
                    fb.setPixel(x, y, pixelColor);
                }
            }
            
        } else if (std::string(argv[1]) == "multipoint") {
            if (argc < 6) {
                std::cerr << "Multipoint mode requires at least 3 points" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            
            int width = std::stoi(argv[2]);
            int height = std::stoi(argv[3]);
            
            // Validate dimensions
            if (width <= 0 || height <= 0) {
                std::cerr << "Error: width and height must be positive integers" << std::endl;
                return 1;
            }
            
            // Parse color points
            std::vector<ColorPoint> points;
            for (int i = 4; i < argc; ++i) {
                try {
                    points.push_back(parseColorPoint(argv[i]));
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing point " << (i - 3) << ": " << e.what() << std::endl;
                    return 1;
                }
            }
            
            if (points.size() < 2) {
                std::cerr << "Error: need at least 2 points for multipoint gradient" << std::endl;
                return 1;
            }
            
            // Create PNG image using inverse distance weighting

            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    // Calculate weighted average of colors using inverse distance weighting
                    float px = static_cast<float>(x);
                    float py = static_cast<float>(y);
                    
                    vec3 pixelColor(0.0f, 0.0f, 0.0f);
                    float totalWeight = 0.0f;
                    
                    // Small epsilon to avoid division by zero when pixel is exactly on a point
                    const float epsilon = 1e-6f;
                    
                    for (const auto& point : points) {
                        float dx = px - point.x;
                        float dy = py - point.y;
                        float distSq = dx * dx + dy * dy;
                        float dist = std::sqrt(distSq);
                        
                        // Use inverse distance as weight (closer = higher weight)
                        // If exactly on point, use that color exclusively
                        float weight;
                        if (dist < epsilon) {
                            pixelColor = point.color;
                            totalWeight = 1.0f;
                            break;  // If on exact point, use only that color
                        } else {
                            weight = 1.0f / dist;
                        }
                        
                        pixelColor = pixelColor + point.color * weight;
                        totalWeight += weight;
                    }
                    
                    // Normalize to get weighted average
                    if (totalWeight > 0.0f) {
                        pixelColor = pixelColor / totalWeight;
                    }

                    fb.setPixel(x, y, pixelColor);
                }
            }

        } else if (std::string(argv[1]) == "solid") {
            if (argc != 5) {
                printUsage(argv[0]);
                return 1;
            }
            
            int width = std::stoi(argv[2]);
            int height = std::stoi(argv[3]);
            std::string hexColor = argv[4];
            
            // Validate dimensions
            if (width <= 0 || height <= 0) {
                std::cerr << "Error: width and height must be positive integers" << std::endl;
                return 1;
            }
            
            // Parse color
            vec3 color = parseHexColor(hexColor);
            
            // Create frame buffer and set background
            fb.setBackground(color);
        } else {
            // Unknown command
            std::cerr << "Unknown command: " << argv[1] << std::endl;
            printUsage(argv[0]);
            return 1;
        }

        // Write to stdout
        char tmpfile[] = "/tmp/pngWriter_XXXXXX";
        int tmpfd = mkstemp(tmpfile);
        if (tmpfd < 0) {
            std::cerr << "Error creating temporary file" << std::endl;
            return 1;
        }
        close(tmpfd);
        
        try {
            fb.writeToPng(tmpfile);
            
            std::ifstream infile(tmpfile, std::ios::binary);
            if (!infile) {
                std::cerr << "Error reading temporary PNG file" << std::endl;
                std::remove(tmpfile);
                return 1;
            }
            
            std::cout << infile.rdbuf();
            infile.close();
            
            std::remove(tmpfile);
        } catch (const std::exception& e) {
            std::remove(tmpfile);
            throw;
        }
        
        return 0;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        printUsage(argv[0]);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}