/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef CHROMA_KEY_HH
#define CHROMA_KEY_HH

#include <condition_variable>
#include <mutex>
#include <thread>

#include "util/dilate_erode.hh"
#include "util/keying.hh"
#include "util/raster.hh"

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
    Keying = 1,
    DilateErodeIntermediate = 2,
    DilateErodeFinal = 3,
    End = 4
  };
  std::vector<Level> output_level_;
  bool output_complete_ { false };

  RGBRaster* raster_ { nullptr };

  RGBRaster& raster() { return *raster_; }
  void process_rows( const uint8_t id );
  // Only return when all the threads completed their previous work
  void synchronize_threads( const uint8_t id, const Level level );

  ChromaKey( const ChromaKey& ) = delete;
  ChromaKey& operator=( const ChromaKey& ) = delete;

public:
  ChromaKey( const uint8_t thread_count,
             const uint16_t width,
             const uint16_t height );
  ~ChromaKey();
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
  void create_mask( RGBRaster& raster );
  void update_color( RGBRaster& raster );
};

#endif /* CHROMA_KEY_HH */
