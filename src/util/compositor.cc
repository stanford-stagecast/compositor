/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>

#include "compositor.hh"

using namespace std;

Compositor::Compositor( const uint8_t num_rasters,
                        const uint16_t width,
                        const uint16_t height,
                        const uint8_t thread_count )
  : width_( width )
  , height_( height )
  , rasters_( num_rasters + 1, nullptr )
  , num_rasters_( num_rasters )
  , keying_modules_( num_rasters, { thread_count, width, height } )
{}

void Compositor::add_raster( RGBRaster* raster, const uint8_t depth )
{
  if ( !raster ) {
    cerr << "Error: Failed to add empty raster to the compositor" << endl;
    return;
  }
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to add raster at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  rasters_[depth] = raster;
}

void Compositor::add_background( RGBRaster* raster )
{
  rasters_[rasters_.size() - 1] = raster;
}

void Compositor::remove_raster( const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to remove raster at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  rasters_[depth] = nullptr;
}

void Compositor::remove_all_rasters()
{
  for ( RGBRaster*& raster : rasters_ ) {
    raster = nullptr;
  }
}

void Compositor::set_dilate_erode_distance( const int distance,
                                            const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to set distance at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  keying_modules_[depth].set_dilate_erode_distance( distance );
}

void Compositor::set_key_color( const std::vector<double>& key_color,
                                const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to set key color at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  keying_modules_[depth].set_key_color( key_color );
}

void Compositor::set_screen_balance( const double screen_balance,
                                     const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to set screen balance at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  keying_modules_[depth].set_screen_balance( screen_balance );
}

void Compositor::set_dilate_erode_distance_all( const int distance )
{
  for ( ChromaKey& keying_module : keying_modules_ ) {
    keying_module.set_dilate_erode_distance( distance );
  }
}

void Compositor::set_key_color_all( const std::vector<double>& key_color )
{
  for ( ChromaKey& keying_module : keying_modules_ ) {
    keying_module.set_key_color( key_color );
  }
}

void Compositor::set_screen_balance_all( const double screen_balance )
{
  for ( ChromaKey& keying_module : keying_modules_ ) {
    keying_module.set_screen_balance( screen_balance );
  }
}

void Compositor::process_pixel( RGBRaster& output_raster,
                                const uint16_t row,
                                const uint16_t col )
{
  double remaining_alpha = 1.0;
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
    output_raster.R().at( col, row ) += alpha * raster->R().at( col, row );
    output_raster.G().at( col, row ) += alpha * raster->G().at( col, row );
    output_raster.B().at( col, row ) += alpha * raster->B().at( col, row );
    remaining_alpha -= alpha;
    if ( remaining_alpha == 0 ) {
      return;
    }
  }
}

void Compositor::process_rows( RGBRaster& output_raster,
                               const uint16_t row_start_idx,
                               const uint16_t row_end_idx )
{
  for ( int row = row_start_idx; row < row_end_idx; row++ ) {
    for ( int col = 0; col < output_raster.width(); col++ ) {
      process_pixel( output_raster, row, col );
    }
  }
}

void Compositor::extract_masks()
{
  vector<int> occupied_indices;
  // Start extracting masks
  for ( int i = 0; i < num_rasters_; i++ ) {
    RGBRaster* raster = rasters_[i];
    if ( !raster ) {
      continue;
    }
    keying_modules_[i].start_create_mask( *raster );
    occupied_indices.push_back( i );
  }
  // Wait for masks to be extracted
  for ( int i : occupied_indices ) {
    keying_modules_[i].wait_for_mask();
  }
}

RGBRaster Compositor::composite()
{
  RGBRaster output_raster { width_, height_, width_, height_ };
  extract_masks();
  process_rows( output_raster, 0, height_ );
  return output_raster;
}
