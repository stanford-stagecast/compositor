/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <cmath>
#include <iostream>

#include "dilate_erode.hh"

using namespace std;

DilateErodeOperation::DilateErodeOperation( const uint16_t width,
                                            const uint16_t height,
                                            const int distance )
  : width_( width )
  , height_( height )
{
  set_distance( distance );
}

void DilateErodeOperation::set_distance( const int distance )
{
  distance_ = distance;
  is_dilation_ = distance_ > 0;
  if ( distance_ < 0 ) {
    distance_ = -distance_;
  }
}

double DilateErodeOperation::process_pixel_intermediate( TwoD<uint8_t>& mask,
                                                         int x,
                                                         int y )
{
  int mask_xmin = 0;
  int mask_xmax = mask.width();
  const int minx = max( x - distance_, mask_xmin );
  const int maxx = min( x + distance_, mask_xmax );
  uint8_t value = is_dilation_ ? 0 : 255;

  for ( int xi = minx; xi < maxx; xi++ ) {
    if ( is_dilation_ ) {
      value = max( mask.at( xi, y ), value );
    } else {
      value = min( mask.at( xi, y ), value );
    }
  }
  return value;
}

double DilateErodeOperation::process_pixel_final( int x, int y )
{
  int mask_ymin = 0;
  int mask_ymax = intermediate_mask_.height();
  const int miny = max( y - distance_, mask_ymin );
  const int maxy = min( y + distance_, mask_ymax );
  uint8_t value = is_dilation_ ? 0 : 255;

  for ( int yi = miny; yi < maxy; yi++ ) {
    if ( is_dilation_ ) {
      value = max( intermediate_mask_.at( x, yi ), value );
    } else {
      value = min( intermediate_mask_.at( x, yi ), value );
    }
  }
  return value;
}

void DilateErodeOperation::process_rows_intermediate(
  TwoD<uint8_t>& mask,
  const uint16_t row_start_idx,
  const uint16_t row_end_idx )
{
  if ( distance_ == 0 ) {
    return;
  }
  for ( uint16_t row = row_start_idx; row < row_end_idx; row++ ) {
    for ( uint16_t col = 0; col < mask.width(); col++ ) {
      intermediate_mask_.at( col, row )
        = process_pixel_intermediate( mask, col, row );
    }
  }
}

void DilateErodeOperation::process_rows_final( TwoD<uint8_t>& mask,
                                               const uint16_t row_start_idx,
                                               const uint16_t row_end_idx )
{
  if ( distance_ == 0 ) {
    return;
  }
  for ( uint16_t row = row_start_idx; row < row_end_idx; row++ ) {
    for ( uint16_t col = 0; col < intermediate_mask_.width(); col++ ) {
      mask.at( col, row ) = process_pixel_final( col, row );
    }
  }
}