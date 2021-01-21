/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>

#include "thread_pool.hh"
#include "util/chroma_key.hh"
#include "util/compositor.hh"

using namespace std;

template<class Module>
ThreadPool<Module>::ThreadPool( const uint8_t thread_count,
                                const uint16_t width,
                                const uint16_t height,
                                Module* module )
  : width_( width )
  , height_( height )
  , module_( module )
  , thread_count_( thread_count )
  , threads_( thread_count_ )
  , output_level_( thread_count_, Start )
{
  for ( uint8_t i = 0; i < thread_count_; i++ ) {
    threads_[i] = std::thread( &ThreadPool::process_rows, this, i );
  }
}

template<class Module>
ThreadPool<Module>::~ThreadPool()
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

template<class Module>
void ThreadPool<Module>::synchronize_threads( const uint8_t id,
                                              const int level )
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

template<class Module>
void ThreadPool<Module>::process_rows( const uint8_t id )
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

    // Do the queued up tasks with synchronization
    for ( size_t i = 0; i < task_list_.size(); i++ ) {
      auto task = task_list_[i];
      // This level corresponds to the current task, since 0 is Start
      const int sync_level = i + 1;
      task( *module_, row_start_idx, row_end_idx );
      synchronize_threads( id, sync_level );
    }

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

template<class Module>
void ThreadPool<Module>::append_task(
  function<void( Module&, const uint16_t, const uint16_t )> task )
{
  task_list_.push_back( task );
  End = task_list_.size() + 1;
}

template<class Module>
void ThreadPool<Module>::input_complete()
{
  {
    lock_guard<mutex> lock( lock_ );
    input_ready_ = true;
  }
  cv_threads_.notify_all();
}

template<class Module>
void ThreadPool<Module>::wait_for_result()
{
  unique_lock<mutex> lock( lock_ );
  cv_main_.wait( lock, [&] {
    return output_complete_
           && accumulate( output_level_.begin(), output_level_.end(), 0 )
                == thread_count_ * End;
  } );
  // Reset all internal states
  output_complete_ = false;
  input_ready_ = false;
  for ( int& level : output_level_ ) {
    level = Start;
  }
}

template class ThreadPool<ChromaKey>;
template class ThreadPool<Compositor>;