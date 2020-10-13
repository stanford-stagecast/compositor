/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <cmath>
#include <iostream>

#include "dilate_erode.hh"

using namespace std;

DilateErodeOperation::DilateErodeOperation( const uint16_t width,
                                            const uint16_t height,
                                            const double distance )
  : width_( width )
  , height_( height )
  , distance_( distance )
{
  if ( distance_ < 0 ) {
    distance_ = -distance_;
  }
  scope_ = distance_;
  if ( scope_ < 3 ) {
    scope_ = 3;
  }
}

double DilateErodeOperation::process_pixel( int x, int y )
{
  const double mindist = distance_ * distance_;

  int mask_xmin = 0;
  int mask_ymin = 0;
  int mask_xmax = old_mask_.width();
  int mask_ymax = old_mask_.height();
  const int minx = max( x - scope_, mask_xmin );
  const int miny = max( y - scope_, mask_ymin );
  const int maxx = min( x + scope_, mask_xmax );
  const int maxy = min( y + scope_, mask_ymax );
  uint8_t value = is_dilation_ ? 0 : 255;

  for ( int yi = miny; yi < maxy; yi++ ) {
    const double dy = yi - y;
    for ( int xi = minx; xi < maxx; xi++ ) {
      const double dx = xi - x;
      const double dis = dx * dx + dy * dy;
      if ( dis <= mindist ) {
        if ( is_dilation_ ) {
          value = max( old_mask_.at( xi, yi ), value );
        } else {
          value = min( old_mask_.at( xi, yi ), value );
        }
      }
    }
  }
  return value;
}

void DilateErodeOperation::init_operation( TwoD<uint8_t>& mask )
{
  if ( distance_ == 0 ) {
    return;
  }
  memcpy( &old_mask_.at( 0, 0 ), &mask.at( 0, 0 ), width_ * height_ );
}

void DilateErodeOperation::process_rows( TwoD<uint8_t>& mask,
                                         const uint16_t row_start_idx,
                                         const uint16_t row_end_idx )
{
  if ( distance_ == 0 ) {
    return;
  }
  for ( uint16_t row = row_start_idx; row < row_end_idx; row++ ) {
    for ( uint16_t col = 0; col < old_mask_.width(); col++ ) {
      mask.at( col, row ) = process_pixel( col, row );
    }
  }
}
