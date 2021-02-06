/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <cmath>
#include <iostream>

#include "despill.hh"

using namespace std;

DespillOperation::DespillOperation( const KeyingOperation& keying_operation )
  : keying_operation_ { keying_operation }
{}

void DespillOperation::process_pixel( RGBRaster& raster, int x, int y )
{
  const vector<double>& key_color
    = keying_operation_.get_key_color( raster, x, y );
  const int screen_primary_channel = keying_operation_.max_axis_v3( key_color );
  const int other_1 = ( screen_primary_channel + 1 ) % 3;
  const int other_2 = ( screen_primary_channel + 2 ) % 3;

  const int min_channel = min( other_1, other_2 );
  const int max_channel = max( other_1, other_2 );

  vector<double> pixel_color { raster.R().at( x, y ) / 255.,
                               raster.G().at( x, y ) / 255.,
                               raster.B().at( x, y ) / 255. };

  const double average_value
    = color_balance_ * pixel_color[min_channel]
      + ( 1.0 - color_balance_ ) * pixel_color[max_channel];
  const double amount = pixel_color[screen_primary_channel] - average_value;

  const double amount_despill = despill_factor_ * amount;
  if ( amount_despill > 0 ) {
    pixel_color[screen_primary_channel] -= amount_despill;
    raster.R().at( x, y ) = pixel_color[0] * 255;
    raster.G().at( x, y ) = pixel_color[1] * 255;
    raster.B().at( x, y ) = pixel_color[2] * 255;
  }
}

void DespillOperation::process_rows( RGBRaster& raster,
                                     const uint16_t row_start_idx,
                                     const uint16_t row_end_idx )
{
  if ( despill_factor_ == 0 ) {
    return;
  }
  for ( int row = row_start_idx; row < row_end_idx; row++ ) {
    for ( int col = 0; col < raster.width(); col++ ) {
      process_pixel( raster, col, row );
    }
  }
}
