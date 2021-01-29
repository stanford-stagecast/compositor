/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef CHROMA_KEY_HH
#define CHROMA_KEY_HH

#include <condition_variable>
#include <mutex>
#include <thread>

#include "util/dilate_erode.hh"
#include "util/keying.hh"
#include "util/raster.hh"
#include "util/thread_pool.hh"

class ChromaKey
{
private:
  // For internal operations
  uint16_t width_, height_;

  const int default_distance_ { 0 };
  const double default_screen_balance_ { 0.5 };
  const std::vector<double> default_key_color_ { 0, 0, 0 };
  KeyingOperation keying_operation_ { default_screen_balance_,
                                      default_key_color_ };
  DilateErodeOperation dilate_erode_operation_ { width_,
                                                 height_,
                                                 default_distance_ };
  int thread_count_;
  ThreadPool<ChromaKey> pool_;
  RGBRaster* raster_ { nullptr };

  RGBRaster& raster() { return *raster_; }
  void keying_task( const uint16_t row_start_idx, const uint16_t row_end_idx );
  void DE_intermediate_task( const uint16_t row_start_idx,
                             const uint16_t row_end_idx );
  void DE_final_task( const uint16_t row_start_idx,
                      const uint16_t row_end_idx );
  ChromaKey& operator=( const ChromaKey& ) = delete;

public:
  ChromaKey( const uint16_t width,
             const uint16_t height,
             const uint8_t thread_count );
  ChromaKey( const ChromaKey& other );
  void set_dilate_erode_distance( const int distance )
  {
    dilate_erode_operation_.set_distance( distance );
  }
  void set_key_color( const std::vector<double>& key_color )
  {
    keying_operation_.set_key_color( key_color );
  }
  void set_screen_balance( const double screen_balance )
  {
    keying_operation_.set_screen_balance( screen_balance );
  }
  void set_marker_config( const int num_horizontal, const int num_vertical )
  {
    keying_operation_.set_marker_config( num_horizontal, num_vertical );
  }
  void set_multikey_color( const RGBRaster& background )
  {
    keying_operation_.set_multikey_color( background );
  }
  void start_create_mask( RGBRaster& raster );
  void wait_for_mask();
  void update_color( RGBRaster& raster );
};

#endif /* CHROMA_KEY_HH */
