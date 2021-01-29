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

  // Multiple key colors option
  std::vector<std::vector<double>> multikey_color_ {};
  int horizontal_num_markers_ { 4 };
  int vertical_num_markers_ = { 2 };
  const std::vector<double>& pixel_to_key_color( const RGBRaster& raster,
                                                 const uint16_t col,
                                                 const uint16_t row ) const;

  int max_axis_v3( const std::vector<double>& vec3 );
  double get_pixel_saturation( const std::vector<double>& pixel_color,
                               int primary_channel );
  double process_pixel( const std::vector<double>& pixel_color,
                        const std::vector<double>& key_color );

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
    multikey_color_.clear();
  }
  void set_marker_config( const int num_horizontal, const int num_vertical )
  {
    horizontal_num_markers_ = num_horizontal;
    vertical_num_markers_ = num_vertical;
  }
  void set_multikey_color( const RGBRaster& background );
  void process_rows( RGBRaster& raster,
                     const uint16_t row_start_idx,
                     const uint16_t row_end_idx );
};

#endif /* KEYING_HH */
