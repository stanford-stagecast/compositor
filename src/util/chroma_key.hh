/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef CHROMA_KEY_HH
#define CHROMA_KEY_HH

#include <vector>

#include "util/raster.hh"

class ChromaKey
{
private:
  double screen_balance_;
  std::vector<double> key_color_;

  int max_axis_v3( const std::vector<double>& vec3 );
  double get_pixel_saturation( const std::vector<double>& pixel_color,
                               int primary_channel );
  double process_pixel( const std::vector<double>& pixel_color );

public:
  ChromaKey( const double screen_balance,
             const std::vector<double>& key_color );
  void create_mask( RGBRaster& raster );
  void update_color( RGBRaster& raster );
};

#endif /* CHROMA_KEY_HH */
