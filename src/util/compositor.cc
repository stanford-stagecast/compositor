/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>
#include <numeric>

#include "compositor.hh"

using namespace std;

Compositor::Compositor( const uint16_t width,
                        const uint16_t height,
                        const uint8_t thread_count )
  : width_( width )
  , height_( height )
  , thread_count_( thread_count )
  , pool_( thread_count, width, height, this )
{
  pool_.append_task( &Compositor::composite_task );
}

void Compositor::composite_pixel( const uint16_t row, const uint16_t col )
{
  double remaining_alpha = 1.0;
  output_raster_.R().at( col, row ) = 0;
  output_raster_.G().at( col, row ) = 0;
  output_raster_.B().at( col, row ) = 0;
  for ( size_t i = 0; i < rasters_.size(); i++ ) {
    RGBRaster* raster = rasters_[i];
    if ( !raster ) {
      continue;
    }
    double raster_alpha = raster->A().at( col, row ) / 255.0;
    // Background doesn't have alpha, so manually set it to 1
    if ( i == rasters_.size() - 1 ) {
      raster_alpha = 1.0;
    }
    const double alpha = min( remaining_alpha, raster_alpha );
    output_raster_.R().at( col, row ) += alpha * raster->R().at( col, row );
    output_raster_.G().at( col, row ) += alpha * raster->G().at( col, row );
    output_raster_.B().at( col, row ) += alpha * raster->B().at( col, row );
    remaining_alpha -= alpha;
    if ( remaining_alpha == 0 ) {
      return;
    }
  }
}

void Compositor::composite_task( const uint16_t row_start_idx,
                                 const uint16_t row_end_idx )
{
  for ( int row = row_start_idx; row < row_end_idx; row++ ) {
    for ( int col = 0; col < output_raster_.width(); col++ ) {
      composite_pixel( row, col );
    }
  }
}

RGBRaster& Compositor::composite()
{
  pool_.input_complete();
  pool_.wait_for_result();
  return output_raster_;
}
