#ifndef UTILITY_WORKER_THREAD_H
#define UTILITY_WORKER_THREAD_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace utility {

class WorkerThread {
 public:
  WorkerThread();
  ~WorkerThread();

  // Adds a task to the queue and returns a future for the result
  std::future<void> add_task(std::function<void()> task);

  // Prevent copy and assignment
  WorkerThread(const WorkerThread&) = delete;
  WorkerThread& operator=(const WorkerThread&) = delete;

  // // Allow move operations
  WorkerThread(WorkerThread&&) noexcept;
  WorkerThread& operator=(WorkerThread&&) noexcept;

 private:
  void start_thread();
  void stop();
  void thread_loop();
  void wait_for_thread_exit();

  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable monitor_;
  bool want_stop_{false};
  std::queue<std::pair<std::function<void()>, std::promise<void>>> task_queue_;
};

}  // namespace utility

#endif  // UTILITY_WORKER_THREAD_H
