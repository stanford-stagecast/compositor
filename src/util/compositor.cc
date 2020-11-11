/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>
#include <numeric>

#include "compositor.hh"

using namespace std;

Compositor::Compositor( const uint8_t num_rasters,
                        const uint16_t width,
                        const uint16_t height,
                        const uint8_t compositor_thread_count,
                        const uint8_t chroma_thread_count )
  : width_( width )
  , height_( height )
  , rasters_( num_rasters + 1, nullptr )
  , num_rasters_( num_rasters )
  , keying_modules_( num_rasters, { chroma_thread_count, width, height } )
  , thread_count_( compositor_thread_count )
  , threads_( thread_count_ )
  , output_level_( thread_count_, Start )
{
  for ( uint8_t i = 0; i < thread_count_; i++ ) {
    threads_[i] = std::thread( &Compositor::process_rows, this, i );
  }
}

Compositor::~Compositor()
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

void Compositor::add_raster( RGBRaster* raster, const uint8_t depth )
{
  if ( !raster ) {
    cerr << "Error: Failed to add empty raster to the compositor" << endl;
    return;
  }
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to add raster at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  rasters_[depth] = raster;
}

void Compositor::add_background( RGBRaster* raster )
{
  rasters_[rasters_.size() - 1] = raster;
}

void Compositor::remove_raster( const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to remove raster at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  rasters_[depth] = nullptr;
}

void Compositor::remove_all_rasters()
{
  for ( RGBRaster*& raster : rasters_ ) {
    raster = nullptr;
  }
}

void Compositor::set_dilate_erode_distance( const int distance,
                                            const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to set distance at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  keying_modules_[depth].set_dilate_erode_distance( distance );
}

void Compositor::set_key_color( const std::vector<double>& key_color,
                                const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to set key color at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  keying_modules_[depth].set_key_color( key_color );
}

void Compositor::set_screen_balance( const double screen_balance,
                                     const uint8_t depth )
{
  if ( depth >= num_rasters_ ) {
    cerr << "Error: Failed to set screen balance at depth " << depth
         << " of range (0 - " << num_rasters_ - 1 << ")" << endl;
    return;
  }
  keying_modules_[depth].set_screen_balance( screen_balance );
}

void Compositor::set_dilate_erode_distance_all( const int distance )
{
  for ( ChromaKey& keying_module : keying_modules_ ) {
    keying_module.set_dilate_erode_distance( distance );
  }
}

void Compositor::set_key_color_all( const std::vector<double>& key_color )
{
  for ( ChromaKey& keying_module : keying_modules_ ) {
    keying_module.set_key_color( key_color );
  }
}

void Compositor::set_screen_balance_all( const double screen_balance )
{
  for ( ChromaKey& keying_module : keying_modules_ ) {
    keying_module.set_screen_balance( screen_balance );
  }
}

void Compositor::process_pixel( const uint16_t row, const uint16_t col )
{
  double remaining_alpha = 1.0;
  output_raster_.R().at( col, row ) = 0;
  output_raster_.G().at( col, row ) = 0;
  output_raster_.B().at( col, row ) = 0;
  for ( size_t i = 0; i < rasters_.size(); i++ ) {
    RGBRaster* raster = rasters_[i];
    if ( !raster ) {
      continue;
    }
    double raster_alpha = raster->A().at( col, row ) / 255.0;
    // Background doesn't have alpha, so manually set it to 1
    if ( i == rasters_.size() - 1 ) {
      raster_alpha = 1.0;
    }
    const double alpha = min( remaining_alpha, raster_alpha );
    output_raster_.R().at( col, row ) += alpha * raster->R().at( col, row );
    output_raster_.G().at( col, row ) += alpha * raster->G().at( col, row );
    output_raster_.B().at( col, row ) += alpha * raster->B().at( col, row );
    remaining_alpha -= alpha;
    if ( remaining_alpha == 0 ) {
      return;
    }
  }
}

void Compositor::synchronize_threads( const uint8_t id, const Level level )
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

void Compositor::process_rows( const uint8_t id )
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

    for ( int row = row_start_idx; row < row_end_idx; row++ ) {
      for ( int col = 0; col < output_raster_.width(); col++ ) {
        process_pixel( row, col );
      }
    }
    synchronize_threads( id, Compose );
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

void Compositor::extract_masks()
{
  vector<int> occupied_indices;
  // Start extracting masks
  for ( int i = 0; i < num_rasters_; i++ ) {
    RGBRaster* raster = rasters_[i];
    if ( !raster ) {
      continue;
    }
    keying_modules_[i].start_create_mask( *raster );
    occupied_indices.push_back( i );
  }
  // Wait for masks to be extracted
  for ( int i : occupied_indices ) {
    keying_modules_[i].wait_for_mask();
  }
}

RGBRaster& Compositor::composite()
{
  extract_masks();
  {
    lock_guard<mutex> lock( lock_ );
    input_ready_ = true;
  }
  cv_threads_.notify_all();

  unique_lock<mutex> lock( lock_ );
  cv_main_.wait( lock, [&] {
    return output_complete_
           && accumulate( output_level_.begin(), output_level_.end(), 0 )
                == thread_count_ * End;
  } );
  // Reset all internal states
  output_complete_ = false;
  input_ready_ = false;
  for ( Level& level : output_level_ ) {
    level = Start;
  }
  return output_raster_;
}
