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
#include "util/thread_pool.hh"

class Compositor
{
private:
  uint16_t width_, height_;
  int thread_count_;
  ThreadPool<Compositor> pool_;
  // The rasters are ordered by depth
  // The raster at index 0 is displayed on top and the last is background
  std::vector<RGBRaster*> rasters_ {};
  RGBRaster output_raster_ { width_, height_, width_, height_ };

  void composite_pixel( const uint16_t row, const uint16_t col );
  void composite_task( const uint16_t row_start_idx,
                       const uint16_t row_end_idx );

public:
  Compositor( const uint16_t width,
              const uint16_t height,
              const uint8_t thread_count = 2 );
  std::vector<RGBRaster*>& raster_list() { return rasters_; }

  RGBRaster& composite();
};

#endif /* COMPOSITOR_HH */
