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
#include "input/mjpeg_input.hh"
#include "util/chroma_key.hh"
#include "util/compositor.hh"
#include "util/h264_encoder.hh"
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
  MJPEGInput frame_input { argv[1], width, height };
  RasterHandle r { RasterHandle { width, height } };
  VideoDisplay display { r, false, false };

  // const uint8_t thread_count = 2;
  // const double distance = 0;
  // const double screen_balance = 0.5;
  // vector<double> key_color = { 131.0 / 255, 204.0 / 255, 160.0 / 255 };
  // ChromaKey chromakey { thread_count, width,          height,
  //                       distance,     screen_balance, key_color };

  // const string image_name = "../test_background.jpg";
  // Compositor compositor( image_name );

  const uint8_t fps = 40;
  H264Encoder encoder( width, height, fps, "ultrafast", "zerolatency" );
  const string output_file = "../test_video_output.h264";
  encoder.create_output_file( output_file );

  while ( true ) {
    auto raster = frame_input.get_next_frame();
    if ( !raster.has_value() ) {
      break;
    }
    auto start = chrono::high_resolution_clock::now();

    // chromakey.create_mask( *raster );
    // chromakey.update_color( *raster );
    // compositor.composite( *raster );

    encoder.encode( *raster );
    EncodedData data = encoder.get_encoded_data();

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>( end - start );
    cout << "Time taken: " << duration.count() << " ms" << endl;

    encoder.write_to_file( data );

    if ( raster.has_value() ) {
      display.draw( *raster );
    }

    this_thread::sleep_for( 40ms );
  }

  return EXIT_SUCCESS;
}
