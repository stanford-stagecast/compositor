/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>

#include "chroma_key.hh"

using namespace std;

ChromaKey::ChromaKey( const uint16_t width,
                      const uint16_t height,
                      const uint8_t thread_count )
  : width_( width )
  , height_( height )
  , thread_count_( thread_count )
  , pool_( thread_count, width, height, this )
{
  pool_.append_task( &ChromaKey::keying_task );
  pool_.append_task( &ChromaKey::DE_intermediate_task );
  pool_.append_task( &ChromaKey::DE_final_task );
}

ChromaKey::ChromaKey( const ChromaKey& other )
  : ChromaKey( other.width_, other.height_, other.thread_count_ )
{}

void ChromaKey::keying_task( const uint16_t row_start_idx,
                             const uint16_t row_end_idx )
{
  keying_operation_.process_rows( raster(), row_start_idx, row_end_idx );
}

void ChromaKey::DE_intermediate_task( const uint16_t row_start_idx,
                                      const uint16_t row_end_idx )
{
  dilate_erode_operation_.process_rows_intermediate(
    raster().A(), row_start_idx, row_end_idx );
}

void ChromaKey::DE_final_task( const uint16_t row_start_idx,
                               const uint16_t row_end_idx )
{
  dilate_erode_operation_.process_rows_final(
    raster().A(), row_start_idx, row_end_idx );
}

void ChromaKey::start_create_mask( RGBRaster& raster )
{
  raster_ = &raster;
  pool_.input_complete();
}

void ChromaKey::wait_for_mask()
{
  pool_.wait_for_result();
}

void ChromaKey::update_color( RGBRaster& raster )
{
  for ( int row = 0; row < raster.height(); row++ ) {
    for ( int col = 0; col < raster.width(); col++ ) {
      const double alpha = raster.A().at( col, row ) / 255.0;
      raster.R().at( col, row ) = alpha * raster.R().at( col, row );
      raster.G().at( col, row ) = alpha * raster.G().at( col, row );
      raster.B().at( col, row ) = alpha * raster.B().at( col, row );
    }
  }
}