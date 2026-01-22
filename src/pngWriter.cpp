#include "FrameBuffer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <cstdio>
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

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <width> <height> <hex_color>" << std::endl;
    std::cerr << "  width:     Image width in pixels" << std::endl;
    std::cerr << "  height:    Image height in pixels" << std::endl;
    std::cerr << "  hex_color: RGB color in hex format (e.g., FF0000 or #FF0000 for red)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        // Parse arguments
        int width = std::stoi(argv[1]);
        int height = std::stoi(argv[2]);
        std::string hexColor = argv[3];
        
        // Validate dimensions
        if (width <= 0 || height <= 0) {
            std::cerr << "Error: width and height must be positive integers" << std::endl;
            return 1;
        }
        
        // Parse color
        vec3 color = parseHexColor(hexColor);
        
        // Create frame buffer and set background
        FrameBuffer fb(width, height);
        fb.setBackground(color);
        
        // Create PNG image in memory
        png::image<png::rgb_pixel> image(width, height);
        
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                const vec3& pixelColor = fb(x, y);
                png::byte r = static_cast<png::byte>(std::clamp(pixelColor[0] * 255.0f, 0.0f, 255.0f));
                png::byte g = static_cast<png::byte>(std::clamp(pixelColor[1] * 255.0f, 0.0f, 255.0f));
                png::byte b = static_cast<png::byte>(std::clamp(pixelColor[2] * 255.0f, 0.0f, 255.0f));
                image[y][x] = png::rgb_pixel(r, g, b);
            }
        }
        
        // Write to stdout by using a temporary file
        // (png++ doesn't support direct stdout writing well)
        char tmpfile[] = "/tmp/pngWriter_XXXXXX";
        int tmpfd = mkstemp(tmpfile);
        if (tmpfd < 0) {
            std::cerr << "Error creating temporary file" << std::endl;
            return 1;
        }
        close(tmpfd);
        
        try {
            image.write(tmpfile);
            
            // Read temporary file and output to stdout
            std::ifstream infile(tmpfile, std::ios::binary);
            if (!infile) {
                std::cerr << "Error reading temporary PNG file" << std::endl;
                std::remove(tmpfile);
                return 1;
            }
            
            std::cout << infile.rdbuf();
            infile.close();
            
            // Clean up
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