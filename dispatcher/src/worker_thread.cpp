#include "worker_thread.h"

#include <iostream>
#include <stdexcept>

namespace utility {

WorkerThread::WorkerThread()
{
  start_thread();
}

WorkerThread::~WorkerThread()
{
  stop();
  wait_for_thread_exit();
}

WorkerThread::WorkerThread(WorkerThread&& other) noexcept
    : thread_(std::move(other.thread_)), want_stop_(other.want_stop_)
{
  // Move the task queue and its synchronization primitives
  std::lock_guard<std::mutex> lock(other.mutex_);
  task_queue_ = std::move(other.task_queue_);
}

WorkerThread&
WorkerThread::operator=(WorkerThread&& other) noexcept
{
  if (this != &other) {
    // Stop and clean up current thread
    stop();
    wait_for_thread_exit();

    // Move the thread and state
    thread_ = std::move(other.thread_);
    want_stop_ = other.want_stop_;

    // Move the task queue and its synchronization primitives
    std::lock_guard<std::mutex> lock(other.mutex_);
    task_queue_ = std::move(other.task_queue_);
  }
  return *this;
}

std::future<void>
WorkerThread::add_task(std::function<void()> task)
{
  std::promise<void> promise;
  std::future<void> future = promise.get_future();

  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (want_stop_) {
      throw std::runtime_error("Worker has stopped, cannot add new task");
    }
    task_queue_.emplace(std::move(task), std::move(promise));
  }

  monitor_.notify_one();  // Notify thread that there's a new task
  return future;
}

void
WorkerThread::start_thread()
{
  thread_ = std::thread([this] { thread_loop(); });
}

void
WorkerThread::stop()
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    want_stop_ = true;
  }
  monitor_.notify_one();
}

void
WorkerThread::thread_loop()
{
  while (true) {
    std::pair<std::function<void()>, std::promise<void>> task;

    {
      std::unique_lock<std::mutex> lock(mutex_);
      monitor_.wait(
          lock, [this] { return want_stop_ || !task_queue_.empty(); });

      if (want_stop_ && task_queue_.empty()) {
        std::cout << "Thread is stopping..." << std::endl;
        return;
      }

      std::cout << "Thread woke up and got a task" << std::endl;
      task = std::move(task_queue_.front());
      task_queue_.pop();
    }

    try {
      task.first();             // Execute the task
      task.second.set_value();  // Set the promise as completed
      std::cout << "Task executed successfully" << std::endl;
    }
    catch (...) {
      task.second.set_exception(std::current_exception());  // Handle exceptions
      std::cout << "Exception occurred while executing task" << std::endl;
    }
  }
}

void
WorkerThread::wait_for_thread_exit()
{
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace utility
