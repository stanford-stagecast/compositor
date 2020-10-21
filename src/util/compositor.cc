/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>

#include "compositor.hh"

using namespace std;

Compositor::Compositor( const string& image_name )
  : background_( jpegdec_.load_image( image_name ) )
{}

void Compositor::composite( RGBRaster& raster )
{
  for ( int row = 0; row < raster.height(); row++ ) {
    for ( int col = 0; col < raster.width(); col++ ) {
      const double alpha = raster.A().at( col, row ) / 255.0;
      raster.R().at( col, row )
        = alpha * raster.R().at( col, row )
          + ( 1.0 - alpha ) * background_.R().at( col, row );
      raster.G().at( col, row )
        = alpha * raster.G().at( col, row )
          + ( 1.0 - alpha ) * background_.G().at( col, row );
      raster.B().at( col, row )
        = alpha * raster.B().at( col, row )
          + ( 1.0 - alpha ) * background_.B().at( col, row );
    }
  }
}
