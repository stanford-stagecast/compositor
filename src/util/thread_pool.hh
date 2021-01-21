/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

template<class Module>
class ThreadPool
{
private:
  // For internal operations
  uint16_t width_, height_;

  // For threading
  Module* module_;
  uint8_t thread_count_;
  std::vector<std::thread> threads_;
  std::mutex lock_ {};
  std::condition_variable cv_threads_ {};
  std::condition_variable cv_main_ {};
  bool thread_terminate_ { false };
  bool input_ready_ { false };
  std::vector<std::function<void( Module&, const uint16_t, const uint16_t )>>
    task_list_ {};
  std::vector<int> output_level_;
  bool output_complete_ { false };

  void process_rows( const uint8_t id );
  // Only return when all the threads completed their current level work
  // For N tasks, each task is synchronized based on their level:
  // Start: 0
  // Task1: 1
  // ...
  // End: N + 1
  const int Start { 0 };
  int End { 0 };
  void synchronize_threads( const uint8_t id, const int level );
  ThreadPool( const ThreadPool& ) = delete;
  ThreadPool& operator=( const ThreadPool& ) = delete;

public:
  ThreadPool( const uint8_t thread_count,
              const uint16_t width,
              const uint16_t height,
              Module* module );
  ~ThreadPool();

  void append_task(
    std::function<void( Module&, const uint16_t, const uint16_t )> task );
  // Mark input ready and allow threads to start processing the tasks
  void input_complete();
  // Returns only when all tasks are completed
  void wait_for_result();
};

#endif /* THREAD_POOL_HH */
