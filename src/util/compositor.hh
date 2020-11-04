/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef COMPOSITOR_HH
#define COMPOSITOR_HH

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "input/jpeg.hh"
#include "util/chroma_key.hh"
#include "util/raster.hh"

class Compositor
{
private:
  uint16_t width_, height_;
  // The rasters are ordered by depth
  // The first raster is displayed on top and the last is background
  std::vector<RGBRaster*> rasters_;
  uint8_t num_rasters_;
  std::vector<ChromaKey> keying_modules_;
  RGBRaster output_raster_ { width_, height_, width_, height_ };

  // For threading
  uint8_t thread_count_;
  std::vector<std::thread> threads_;
  std::mutex lock_ {};
  std::condition_variable cv_threads_ {};
  std::condition_variable cv_main_ {};
  bool thread_terminate_ { false };
  bool input_ready_ { false };
  enum Level
  {
    Start = 0,
    Compose = 1,
    End = 2
  };
  std::vector<Level> output_level_;
  bool output_complete_ { false };
  // Only return when all the threads completed their previous work
  void synchronize_threads( const uint8_t id, const Level level );

  RGBRaster& background() { return *rasters_[rasters_.size() - 1]; }
  void process_pixel( const uint16_t row, const uint16_t col );
  void process_rows( const uint8_t id );
  void extract_masks();

public:
  Compositor( const uint8_t num_rasters,
              const uint16_t width,
              const uint16_t height,
              const uint8_t compositor_thread_count = 2,
              const uint8_t chroma_thread_count = 2 );
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

  RGBRaster& composite();
};

#endif /* COMPOSITOR_HH */
