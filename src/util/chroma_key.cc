/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>

#include "chroma_key.hh"

using namespace std;

ChromaKey::ChromaKey( const uint8_t thread_count,
                      const uint16_t width,
                      const uint16_t height )
  : width_( width )
  , height_( height )
  , thread_count_( thread_count )
  , threads_( thread_count_ )
  , output_level_( thread_count_, Start )
{
  for ( uint8_t i = 0; i < thread_count_; i++ ) {
    threads_[i] = std::thread( &ChromaKey::process_rows, this, i );
  }
}

ChromaKey::ChromaKey( const ChromaKey& other )
  : ChromaKey( other.thread_count_, other.width_, other.height_ )
{}

ChromaKey::~ChromaKey()
{
  {
    lock_guard<mutex> lock( lock_ );
    thread_terminate_ = true;
  }
  cv_threads_.notify_all();
  for ( auto& t : threads_ ) {
    t.join();
  }
}

void ChromaKey::synchronize_threads( const uint8_t id, const Level level )
{
  unique_lock<mutex> lock( lock_ );
  output_level_[id] = level;
  // If this is the last thread to finish, wake up all threads, otherwise wait
  // until other threads finished
  if ( accumulate( output_level_.begin(), output_level_.end(), 0 )
       == thread_count_ * level ) {
    // Manually unlock the lock to avoid waking up the waiting threads only to
    // block again
    input_ready_ = false;
    lock.unlock();
    cv_threads_.notify_all();
  } else {
    cv_threads_.wait( lock, [&] {
      return accumulate( output_level_.begin(), output_level_.end(), 0 )
               >= thread_count_ * level
             || ( level == End && output_level_[id] == Start );
    } );
  }
}

void ChromaKey::process_rows( const uint8_t id )
{
  const uint16_t num_rows = height_ / thread_count_;
  const uint16_t row_start_idx = id * num_rows;
  const uint16_t row_end_idx = row_start_idx + num_rows;
  while ( true ) {
    {
      unique_lock<mutex> lock( lock_ );
      cv_threads_.wait( lock,
                        [&] { return input_ready_ || thread_terminate_; } );
      if ( thread_terminate_ ) {
        return;
      }
    } // End of lock scope

    // Keying
    keying_operation_.process_rows( raster(), row_start_idx, row_end_idx );
    synchronize_threads( id, Keying );

    // Dilation or erosion
    dilate_erode_operation_.process_rows_intermediate(
      raster().A(), row_start_idx, row_end_idx );
    synchronize_threads( id, DilateErodeIntermediate );

    dilate_erode_operation_.process_rows_final(
      raster().A(), row_start_idx, row_end_idx );
    synchronize_threads( id, DilateErodeFinal );

    // Ensure only one thread marks the job as done and wakes up main thread
    {
      unique_lock<mutex> lock( lock_ );
      if ( !output_complete_ ) {
        output_complete_ = true;
      }
    } // End of lock scope
    synchronize_threads( id, End );
    cv_main_.notify_one();
  }
}

void ChromaKey::start_create_mask( RGBRaster& raster )
{
  raster_ = &raster;
  {
    lock_guard<mutex> lock( lock_ );
    input_ready_ = true;
  }
  cv_threads_.notify_all();
}

void ChromaKey::wait_for_mask()
{
  {
    unique_lock<mutex> lock( lock_ );
    cv_main_.wait( lock, [&] { return output_complete_; } );
    output_complete_ = false;
  }
  // Reset all internal states
  input_ready_ = false;
  raster_ = nullptr;
  for ( Level& level : output_level_ ) {
    level = Start;
  }
}

void ChromaKey::update_color( RGBRaster& raster )
{
  for ( int row = 0; row < raster.height(); row++ ) {
    for ( int col = 0; col < raster.width(); col++ ) {
      const double alpha = raster.A().at( col, row ) / 255.0;
      raster.R().at( col, row ) = alpha * raster.R().at( col, row );
      raster.G().at( col, row ) = alpha * raster.G().at( col, row );
      raster.B().at( col, row ) = alpha * raster.B().at( col, row );
    }
  }
}