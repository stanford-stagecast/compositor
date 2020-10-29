/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef COMPOSITOR_HH
#define COMPOSITOR_HH

#include <vector>

#include "input/jpeg.hh"
#include "util/chroma_key.hh"
#include "util/raster.hh"

class Compositor
{
private:
  uint16_t width_, height_;
  // The rasters are ordered by depth
  // The first raster is displayed on top and the last is usually background
  std::vector<RGBRaster*> rasters_;
  uint8_t num_rasters_;
  std::vector<ChromaKey> keying_modules_;

  RGBRaster& background() { return *rasters_[rasters_.size() - 1]; }
  void process_pixel( RGBRaster& output_raster,
                      const uint16_t row,
                      const uint16_t col );
  void process_rows( RGBRaster& output_raster,
                     const uint16_t row_start_idx,
                     const uint16_t row_end_idx );
  void extract_masks();

public:
  Compositor( const uint8_t num_rasters,
              const uint16_t width,
              const uint16_t height,
              const uint8_t thread_count = 2 );
  void add_raster( RGBRaster* raster, const uint8_t depth );
  void add_background( RGBRaster* raster );
  void remove_raster( const uint8_t depth );
  void remove_all_rasters();

  void set_dilate_erode_distance( const int distance, const uint8_t depth );
  void set_key_color( const std::vector<double>& key_color,
                      const uint8_t depth );
  void set_screen_balance( const double screen_balance, const uint8_t depth );

  void set_dilate_erode_distance_all( const int distance );
  void set_key_color_all( const std::vector<double>& key_color );
  void set_screen_balance_all( const double screen_balance );

  RGBRaster composite();
};

#endif /* COMPOSITOR_HH */
