/**
 * This module is meant to test the outputs from chromakey run.
 * It does this by using the tru ground version of a video extracted from
 * the blender software and comparing that to the output from chromakey
 *
 * Performance measures are based on levels of compression
 */

#include <getopt.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "display/display.hh"
#include "input/mjpeg_input.hh"
#include "util/chroma_key.hh"
#include "util/raster_handle.hh"

using namespace std;
using namespace chrono;

void usage( const char* argv0 )
{
  cerr << "Usage: " << argv0 << " FILE-ORIGINAL FILE-CURRENT" << endl;
}

int main( int argc, char* argv[] )
{
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  if ( argc != 3 ) {
    usage( argv[0] );
    return EXIT_FAILURE;
  }

  const string original_path { argv[1] };
  const string current_path { argv[2] };
  const uint16_t width = 1280;
  const uint16_t height = 720;

  const double screen_balance = 0.5;
  vector<double> key_color = { 0.00819513, 0.106535, 0.026461 };

  MJPEGInput original_mjpeg_input { original_path, width, height };
  MJPEGInput current_mjpeg_input { current_path, width, height };

  RasterHandle r { RasterHandle { width, height } };
  VideoDisplay display { r, false, true };
  ChromaKey chroma_key { 4, width, height, 0, screen_balance, key_color };

  while ( true ) {
    auto original_raster = original_mjpeg_input.get_next_rgb_frame();
    auto current_raster = current_mjpeg_input.get_next_rgb_frame();

    if ( not original_raster.has_value() or not current_raster.has_value() ) {
      break;
    }

    chroma_key.create_mask( *original_raster );
    chroma_key.create_mask( *current_raster );

    //chroma_key.update_color( *original_raster );
    //display.draw( *original_raster );
    chroma_key.update_color( *current_raster );
    display.draw( *current_raster );
    
    size_t diff = 0;

    // comparing original_raster->get().A() & current_raster->get().A()
    for ( size_t i = 0; i < width; i++ ) {
      for ( size_t j = 0; j < height; j++ ) {
        const auto a1 = original_raster->get().A().at( i, j );
        const auto a2 = current_raster->get().A().at( i, j );

        if ( abs( a1 - a2 ) >= 64 ) {
          diff++;
        }
      }
    }

    cout << ( 1.0 * diff / ( width * height ) ) << endl;
  }

  return EXIT_SUCCESS;
}
