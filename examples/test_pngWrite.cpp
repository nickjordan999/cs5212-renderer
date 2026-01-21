/*
 *  test_pngWrite.cpp
 *
 *  Created by Pete Willemsen on 10/6/09.
 *  Copyright 2009 Department of Computer Science, University of Minnesota-Duluth. All rights reserved.
 *
 * This file is part of CS5721 Computer Graphics library (cs5721Graphics).
 *
 * cs5721Graphics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cs5721Graphics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with cs5721Graphics.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>

#include "png++/png.hpp"
#include "handleGraphicsArgs.h"

int main(int argc, char *argv[])
{
    // pseudo-random generation classes in C++
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::uniform_real_distribution<> real_distrib(0.0f, 255.0f);

    sivelab::GraphicsArgs args;
    args.process(argc, argv);

    // Create a red image
    int w = args.width, h = args.height;
    png::image< png::rgb_pixel > imData( w, h );
    for (size_t y = 0; y < imData.get_height(); ++y)
    {
        for (size_t x = 0; x < imData.get_width(); ++x)
	{
            // non-checking equivalent of image.set_pixel(x, y, ...);
            imData[y][x] = png::rgb_pixel(255, 0, 0);
	}
    }
    imData.write( "red_DualLoop.png" );

    //
    // Alternatively, you can do the same using a single loop:
    //
    for (unsigned int idx=0; idx<imData.get_height()*imData.get_width(); ++idx)
    {
        size_t x = idx % w;
        size_t y = static_cast<size_t>( floor(idx / static_cast<float>(imData.get_width())) );

        // non-checking equivalent of image.set_pixel(x, y, ...);
	imData[y][x] = png::rgb_pixel(0, 255, 0);
    }
    imData.write( "green_SingleLoop.png" );

    //
    // create an image with random colors in every pixel
    //
    for (unsigned int idx=0; idx<imData.get_height()*imData.get_width(); ++idx)
    {
        size_t x = idx % w;
        size_t y = static_cast<size_t>( floor(idx / static_cast<float>(imData.get_width())) );
      
        float c[3] = {float(distrib(gen)),
                      float(distrib(gen)),
                      float(distrib(gen))};
      
        // The origin for indexing the height is in lower left...
        imData[y][x] = png::rgb_pixel( c[0], c[1], c[2] );
    }
    imData.write( "random.png" );

    for (unsigned int idx=0; idx<imData.get_height()*imData.get_width(); ++idx)
    {
        size_t x = idx % w;
        size_t y = static_cast<size_t>( floor(idx / static_cast<float>(imData.get_width())) );

        // radial distance to edge of image
        float max_distance = sqrt( static_cast<float>((w/2.0*w/2.0) + (h/2.0*h/2.0)) );
        float dist = sqrt( static_cast<float>((x - w/2.0)*(x - w/2.0) + (y - h/2.0)*(y - h/2.0)) ) / max_distance;

        png::byte c[3] = {static_cast<png::byte>(std::clamp( dist * 255.0f, 0.0f, 255.0f )),
                          static_cast<png::byte>(std::clamp( dist * 255.0f, 0.0f, 255.0f )),
                          static_cast<png::byte>(std::clamp( dist * 255.0f, 0.0f, 255.0f )) };

        // The origin for indexing the height is in lower left...
        imData[y][x] = png::rgb_pixel( c[0], c[1], c[2] );
    }
    imData.write( "radial_Center.png" );

    // Creates a radial gradient image from the origin of the image.
    // Note that the origin is in the upper left.
    for (unsigned int idx=0; idx<imData.get_height()*imData.get_width(); ++idx)
    {
        size_t x = idx % w;
        size_t y = static_cast<size_t>( floor(idx / static_cast<float>(imData.get_width())) );

        // assert((y >= 0) && (y < h) && x >= 0 && x < w);

        // radial distance to edge of image
        float max_distance = sqrt( static_cast<float>((w*w) + (h*h)) );
        float dist = sqrt( static_cast<float>((x*x) + (y*y)) ) / max_distance;

        png::byte c[3] = {static_cast<png::byte>(std::clamp( dist * 255.0f, 0.0f, 255.0f )),
                          static_cast<png::byte>(std::clamp( dist * 255.0f, 0.0f, 255.0f )),
                          static_cast<png::byte>(std::clamp( dist * 255.0f, 0.0f, 255.0f )) };

        imData[y][x] = png::rgb_pixel( c[0], c[1], c[2] );
    }
  
    imData.write( "radial_ImageOrigin.png" );


    //
    // Checkerboard
    //
    for (unsigned int idx=0; idx<imData.get_height()*imData.get_width(); ++idx)
    {
        size_t x = idx % w;
        size_t y = static_cast<size_t>( floor(idx / static_cast<float>(imData.get_width())) );

        // bitwise AND of each value XOR'd with each other
        int v = (((x & 0x8)==0) ^ ((y & 0x8)==0)) * 255;
        png::byte c[3] = { static_cast<png::byte>(v), static_cast<png::byte>(v), static_cast<png::byte>(v) };
      
        imData[y][x] = png::rgb_pixel( c[0], c[1], c[2] );
    }
    imData.write( "checkerboard.png" );


    //
    // Gradient
    //

    float sr = 1.0f;
    float sg = 0.0f;
    float sb = 0.0f;

    float er = 0.0f;
    float eg = 0.0f;
    float eb = 1.0f;

    for (unsigned int idx=0; idx<imData.get_height()*imData.get_width(); ++idx)
    {
        size_t x = idx % w;
        size_t y = static_cast<size_t>( floor(idx / static_cast<float>(imData.get_width())) );


        float t = y / static_cast<float>(imData.get_height()-1);

        png::byte c[3] = { static_cast<png::byte>( (sr + (er - sr) * t) * 255.0f ),
                           static_cast<png::byte>( (sg + (eg - sg) * t) * 255.0f ),
                           static_cast<png::byte>( (sb + (eb - sb) * t) * 255.0f ) };

        imData[y][x] = png::rgb_pixel( c[0], c[1], c[2] );
    }

    imData.write( "gradient.png" );

    exit(EXIT_SUCCESS);

}
