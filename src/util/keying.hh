/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef KEYING_HH
#define KEYING_HH

#include <vector>

#include "util/raster.hh"

class KeyingOperation
{
private:
  double screen_balance_;
  std::vector<double> key_color_;

  int max_axis_v3( const std::vector<double>& vec3 );
  double get_pixel_saturation( const std::vector<double>& pixel_color,
                               int primary_channel );
  double process_pixel( const std::vector<double>& pixel_color );

public:
  KeyingOperation( const double screen_balance,
                   const std::vector<double>& key_color );
  void set_screen_balance( const double screen_balance )
  {
    screen_balance_ = screen_balance;
  }
  void set_key_color( const std::vector<double>& key_color )
  {
    key_color_ = key_color;
  }
  void process_rows( RGBRaster& raster,
                     const uint16_t row_start_idx,
                     const uint16_t row_end_idx );
};

#endif /* KEYING_HH */
