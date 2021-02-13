/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <cmath>
#include <iostream>

#include "keying_clip.hh"

using namespace std;

double KeyingClipOperation::process_pixel( TwoD<uint8_t>& mask, int x, int y )
{
  const double alpha = mask.at( x, y ) / 255.;
  bool within_tolerance = false;
  const int start_x = max( 0, x - kernel_radius_ + 1 );
  const int start_y = max( 0, y - kernel_radius_ + 1 );
  const int end_x
    = min( x + kernel_radius_ - 1, static_cast<int>( mask.width() ) - 1 );
  const int end_y
    = min( y + kernel_radius_ - 1, static_cast<int>( mask.height() ) - 1 );

  int count = 0;
  int total_count = ( end_x - start_x + 1 ) * ( end_y - start_y + 1 ) - 1;
  int threshold_count = ceil( 0.9 * total_count );

  if ( kernel_radius_ == 0 ) {
    within_tolerance = true;
  }

  for ( int cx = start_x; within_tolerance == false && cx <= end_x; cx++ ) {
    for ( int cy = start_y; within_tolerance == false && cy <= end_y; cy++ ) {
      if ( cx == x && cy == y ) {
        continue;
      }
      const double cur_alpha = mask.at( cx, cy ) / 255.;
      if ( abs( cur_alpha - alpha ) < kernel_tolerance_ ) {
        count++;
        if ( count >= threshold_count ) {
          within_tolerance = true;
        }
      }
    }
  }

  double output = alpha;
  if ( within_tolerance ) {
    if ( output < clip_black_ ) {
      output = 0.;
    } else if ( output >= clip_white_ ) {
      output = 1.;
    } else {
      output = ( output - clip_black_ ) / ( clip_white_ - clip_black_ );
    }
  }
  return output;
}

void KeyingClipOperation::process_rows( TwoD<uint8_t>& mask,
                                        const uint16_t row_start_idx,
                                        const uint16_t row_end_idx )
{
  if ( clip_black_ == 0 && clip_white_ == 1 ) {
    return;
  }
  for ( size_t row = row_start_idx; row < row_end_idx; row++ ) {
    for ( size_t col = 0; col < mask.width(); col++ ) {
      const double alpha = process_pixel( mask, col, row );
      mask.at( col, row ) = alpha * 255;
    }
  }
}
