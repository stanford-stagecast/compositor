/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef DILATE_ERODE_HH
#define DILATE_ERODE_HH

#include "util/raster.hh"

class DilateErodeOperation
{
private:
  uint16_t width_, height_;
  double distance_ {};
  int scope_ {};
  bool is_dilation_ { distance_ > 0 };
  TwoD<uint8_t> old_mask_ { width_, height_ };

  double process_pixel( int x, int y );

public:
  DilateErodeOperation( const uint16_t width,
                        const uint16_t height,
                        const double distance );
  // Copies mask to old_mask_ because convolutions can't be done in place
  void init_operation( TwoD<uint8_t>& mask );
  void set_distance( const double distance );
  void process_rows( TwoD<uint8_t>& mask,
                     const uint16_t row_start_idx,
                     const uint16_t row_end_idx );
};

#endif /* DILATE_ERODE_HH */
