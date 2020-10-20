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
#include "util/chroma_key.hh"
#include "util/raster_handle.hh"

using namespace std;

// TO DO: Once finalised, add instructions on how to run here:
void usage( const char* argv0 ) {
    cerr << "Usage: " << argv0 << endl;
}

int main(int argc, char* argv[] ) {
    
}