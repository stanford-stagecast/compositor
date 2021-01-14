/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef DILATE_ERODE_HH
#define DILATE_ERODE_HH

#include "util/raster.hh"

class DilateErodeOperation
{
private:
  uint16_t width_, height_;
  int distance_ {};
  bool is_dilation_ { distance_ > 0 };
  TwoD<uint8_t> intermediate_mask_ { width_, height_ };

  double process_pixel_intermediate( TwoD<uint8_t>& mask, int x, int y );
  double process_pixel_final( int x, int y );

public:
  DilateErodeOperation( const uint16_t width,
                        const uint16_t height,
                        const int distance );
  void set_distance( const int distance );
  // The kernel is separable, so the convolution is done in two steps for speed
  void process_rows_intermediate( TwoD<uint8_t>& mask,
                                  const uint16_t row_start_idx,
                                  const uint16_t row_end_idx );
  void process_rows_final( TwoD<uint8_t>& mask,
                           const uint16_t row_start_idx,
                           const uint16_t row_end_idx );
};

#endif /* DILATE_ERODE_HH */
