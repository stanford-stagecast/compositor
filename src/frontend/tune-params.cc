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
#include "input/camera.hh"
#include "input/jpeg.hh"
#include "util/chroma_key.hh"
#include "util/compositor.hh"
#include "util/raster_handle.hh"
#include "util/tokenize.hh"

using namespace std;

void usage( const char* argv0 )
{
  cerr
    << "Usage: " << argv0
    << " [-d, --device CAMERA] [-p, --pixfmt PIXEL_FORMAT] [-f, --fullscreen]"
    << endl;
}

int main( int argc, char* argv[] )
{
  /* check the command-line arguments */
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  /* camera settings */
  string camera_device = "/dev/video0";
  string pixel_format = "NV12";
  bool fullscreen = false;

  const option command_line_options[]
    = { { "device", required_argument, nullptr, 'd' },
        { "pixfmt", required_argument, nullptr, 'p' },
        { "fullscreen", no_argument, nullptr, 'f' },
        { 0, 0, 0, 0 } };

  while ( true ) {
    const int opt
      = getopt_long( argc, argv, "d:p:f", command_line_options, nullptr );

    if ( opt == -1 ) {
      break;
    }

    switch ( opt ) {
      case 'd':
        camera_device = optarg;
        break;
      case 'p':
        pixel_format = optarg;
        break;
      case 'f':
        fullscreen = true;
        break;

      default:
        usage( argv[0] );
        return EXIT_FAILURE;
    }
  }

  const uint16_t width = 1280;
  const uint16_t height = 720;
  Camera camera {
    width, height, PIXEL_FORMAT_STRS.at( pixel_format ), camera_device
  };

  RasterHandle r { RasterHandle { width, height } };

  VideoDisplay original_display { r, fullscreen, true };
  VideoDisplay output_display { r, fullscreen, true };

  const uint8_t thread_count = 2;
  const int distance = 0;
  const double screen_balance = 0.5;
  vector<double> key_color = { 0.00819513, 0.106535, 0.026461 };
  ChromaKey chromakey { thread_count, width, height };
  chromakey.set_key_color( key_color );
  chromakey.set_screen_balance( screen_balance );
  chromakey.set_dilate_erode_distance( distance );

  const string image_name = "../test_background.jpg";
  JPEGDecompresser jpegdec;
  RGBRaster background = jpegdec.load_image( image_name );

  thread display_thread( [&] {
    while ( true ) {
      auto raster = camera.get_next_rgb_frame();

      if ( raster.has_value() ) {
        original_display.draw( *raster );
      }

      chromakey.start_create_mask( *raster );
      chromakey.wait_for_mask();

      chromakey.update_color( *raster );
      if ( raster.has_value() ) {
        output_display.draw( *raster );
      }
    }
  } );

  thread command_thread( [&] {
    while ( true ) {
      string command;
      getline( cin, command );
      auto tokens = split( command, " " );

      if ( tokens[0] == "balance" ) {
        const double balance = stof( tokens[1] );
        chromakey.set_screen_balance( balance );
      } else if ( tokens[0] == "color" ) {
        if ( tokens.size() != 4 ) {
          cerr << "Not enough color components" << endl;
          continue;
        }

        const int R = stol( tokens[1] );
        const int G = stol( tokens[2] );
        const int B = stol( tokens[3] );

        const vector<double> color { R / 255.0, G / 255.0, B / 255.0 };
        chromakey.set_key_color( color );
      }
    }
  } );

  command_thread.join();
  display_thread.join();

  return EXIT_SUCCESS;
}
