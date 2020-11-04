/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* Copyright 2013-2018 the Alfalfa authors
                       and the Massachusetts Institute of Technology

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

      1. Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <getopt.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "display/display.hh"
#include "input/jpeg.hh"
#include "input/mjpeg_input.hh"
#include "util/chroma_key.hh"
#include "util/compositor.hh"
#include "util/raster_handle.hh"

using namespace std;
using namespace chrono;

void usage( const char* argv0 )
{
  cerr << "Usage: " << argv0 << " FILE" << endl;
}

int main( int argc, char* argv[] )
{
  /* check the command-line arguments */
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  /* camera settings */
  const uint16_t width = 1280;
  const uint16_t height = 720;
  MJPEGInput frame_input1 { argv[1], width, height };
  MJPEGInput frame_input2 { argv[2], width, height };
  RasterHandle r { RasterHandle { width, height } };
  VideoDisplay display { r, false, true };

  const uint8_t thread_count = 2;
  // const int distance = 0;
  // const double screen_balance = 0.5;
  vector<double> key_color1
    = { 133.0 / 255, 187.0 / 255, 119.0 / 255 }; // puncher
  vector<double> key_color2
    = { 169.0 / 255, 220.0 / 255, 125.0 / 255 }; // punchee

  const string image_name = "../test_background.jpg";
  JPEGDecompresser jpegdec;
  RGBRaster background = jpegdec.load_image( image_name );
  Compositor compositor( 3, width, height, thread_count );
  compositor.set_key_color( key_color1, 0 );
  compositor.set_key_color( key_color2, 1 );
  compositor.add_background( &background );

  for ( int i = 0; i < 70; i++ ) {
    frame_input1.get_next_rgb_frame();
  }

  while ( true ) {
    auto raster1 = frame_input1.get_next_rgb_frame();
    auto raster2 = frame_input2.get_next_rgb_frame();
    if ( !raster1.has_value() && !raster2.has_value() ) {
      break;
    }
    if ( raster1.has_value() ) {
      compositor.add_raster( &raster1.value().get(), 0 );
    }
    if ( raster2.has_value() ) {
      compositor.add_raster( &raster2.value().get(), 1 );
    }
    auto start = chrono::high_resolution_clock::now();

    RGBRaster& output_raster = compositor.composite();

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>( end - start );
    cout << "Time taken: " << duration.count() << " ms" << endl;

    display.draw( output_raster );

    this_thread::sleep_for( 40ms );
  }

  return EXIT_SUCCESS;
}
