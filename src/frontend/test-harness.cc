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
  cerr << "Usage: " << argv0 << " FILE WIDTH HEIGHT" << endl;
}

int main( int argc, char* argv[] )
{
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  if ( argc != 4 ) {
    usage( argv[0] );
    return EXIT_FAILURE;
  }

  const string path { argv[1] };
  const uint16_t width = stoi( argv[2] );
  const uint16_t height = stoi( argv[3] );

  MJPEGInput mjpeg_input { path, width, height };
  RasterHandle r { RasterHandle { width, height } };
  VideoDisplay display { r, false, true };

  while ( true ) {
    auto next_frame = steady_clock::now() + 100ms;
    auto raster = mjpeg_input.get_next_rgb_frame();

    if ( raster.has_value() ) {
      display.draw( *raster );
    } else {
      break;
    }

    this_thread::sleep_until( next_frame );
  }

  return EXIT_SUCCESS;
}
