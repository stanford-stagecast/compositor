/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef KEYING_CLIP_HH
#define KEYING_CLIP_HH

#include "util/raster.hh"

class KeyingClipOperation
{
private:
  uint8_t kernel_radius_ { 0 };
  double kernel_tolerance_ { 0.1 };
  double clip_black_ { 0. };
  double clip_white_ { 1. };

  double process_pixel( TwoD<uint8_t>& mask, int x, int y );

public:
  void set_kernel_radius( const uint8_t radius ) { kernel_radius_ = radius; }
  void set_kernel_tolerance( const double tolerance )
  {
    kernel_tolerance_ = tolerance;
  }
  void set_clip_black( const double clip_black ) { clip_black_ = clip_black; }
  void set_clip_white( const double clip_white ) { clip_white_ = clip_white; }
  void process_rows( TwoD<uint8_t>& mask,
                     const uint16_t row_start_idx,
                     const uint16_t row_end_idx );
};

#endif /* KEYING_CLIP_HH */
