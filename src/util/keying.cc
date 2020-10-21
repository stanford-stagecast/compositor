/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>

#include "keying.hh"

using namespace std;

KeyingOperation::KeyingOperation( const double screen_balance,
                                  const vector<double>& key_color )
  : screen_balance_( screen_balance )
  , key_color_( key_color )
{}

int KeyingOperation::max_axis_v3( const std::vector<double>& vec3 )
{
  const double x = vec3[0];
  const double y = vec3[1];
  const double z = vec3[2];
  return ( ( x > y ) ? ( ( x > z ) ? 0 : 2 ) : ( ( y > z ) ? 1 : 2 ) );
}

double KeyingOperation::get_pixel_saturation( const vector<double>& pixel_color,
                                              int primary_channel )
{
  const int other_1 = ( primary_channel + 1 ) % 3;
  const int other_2 = ( primary_channel + 2 ) % 3;

  const int min_channel = min( other_1, other_2 );
  const int max_channel = max( other_1, other_2 );

  const double val = screen_balance_ * pixel_color[min_channel]
                     + ( 1.0 - screen_balance_ ) * pixel_color[max_channel];

  return ( pixel_color[primary_channel] - val ) * abs( 1.0 - val );
}

double KeyingOperation::process_pixel( const vector<double>& pixel_color )
{
  double alpha = 0;
  const int primary_channel = max_axis_v3( key_color_ );
  const double min_pixel_color
    = min( min( pixel_color[0], pixel_color[1] ), pixel_color[2] );

  if ( min_pixel_color > 1.0 ) {
    /* Overexposure doesn't happen on screen itself and usually happens
     * on light sources in the shot, this need to be checked separately
     * because saturation and falloff calculation is based on the fact
     * that pixels are not overexposed
     */
    alpha = 1.0;
  } else {
    double saturation = get_pixel_saturation( pixel_color, primary_channel );
    double screen_saturation
      = get_pixel_saturation( key_color_, primary_channel );

    if ( saturation < 0 ) {
      // Means main channel of pixel is different from screen, assume this is
      // completely a foreground
      alpha = 1.0;
    } else if ( saturation >= screen_saturation ) {
      // Matched main channels and higher saturation on pixel is treated as
      // completely background
      alpha = 0.0;
    } else {
      // Nice alpha falloff on edges
      double distance = 1.0 - saturation / screen_saturation;
      alpha = distance;
    }
  }
  return alpha;
}

void KeyingOperation::process_rows( RGBRaster& raster,
                                    const uint16_t row_start_idx,
                                    const uint16_t row_end_idx )
{
  for ( int row = row_start_idx; row < row_end_idx; row++ ) {
    for ( int col = 0; col < raster.width(); col++ ) {
      double r = raster.R().at( col, row );
      double g = raster.G().at( col, row );
      double b = raster.B().at( col, row );
      vector<double> pixel_color = { r / 255.0, g / 255.0, b / 255.0 };
      double alpha = process_pixel( pixel_color );
      raster.A().at( col, row ) = alpha * 255;
    }
  }
}
