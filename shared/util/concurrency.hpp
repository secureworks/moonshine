#ifndef MOONSHINE_SHARED_UTIL_CONCURRENCY_H_
#define MOONSHINE_SHARED_UTIL_CONCURRENCY_H_

#include <queue>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace util {

namespace concurrency {

struct deque_stopped : public std::runtime_error {
  deque_stopped() : std::runtime_error("The queue has stopped") {}
};

template<typename T>
struct deque {

  deque() : deque_has_stopped_{false} {}

  deque(const deque &) = delete;            // disable copying
  deque &operator=(const deque &) = delete; // disable assignment

  bool try_pop_all(std::vector<T> &items) {
    std::unique_lock lock(mutex_);
    if (deque_.empty() || deque_has_stopped_)
      return false;
    while (!deque_.empty()) {
      items.push_back(deque_.front());
      deque_.pop_front();
    }
    return true;
  }

  [[nodiscard]] T wait_and_pop_front() {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&] { return !deque_.empty() || deque_has_stopped_; });
    if (deque_has_stopped_)
      throw deque_stopped();
    auto val = deque_.front();
    deque_.pop_front();
    lock.unlock();
    return val;
  }

  void wait_and_pop_front(T &item) {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&] { return !deque_.empty() || deque_has_stopped_; });
    if (deque_has_stopped_)
      throw deque_stopped();
    item = deque_.front();
    deque_.pop_front();
  }

  bool try_pop_front(T &item) {
    std::unique_lock lock(mutex_);
    if (deque_.empty() || deque_has_stopped_)
      return false;
    item = deque_.front();
    deque_.pop_front();
    return true;
  }

  [[nodiscard]] T wait_and_pop_back() {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&] { return !deque_.empty() || deque_has_stopped_; });
    if (deque_has_stopped_)
      throw deque_stopped();
    auto val = deque_.back();
    deque_.pop_back();
    lock.unlock();
    return val;
  }

  void wait_and_pop_back(T &item) {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&] { return !deque_.empty() || deque_has_stopped_; });
    if (deque_has_stopped_)
      throw deque_stopped();
    item = deque_.back();
    deque_.pop_back();
  }

  bool try_pop_back(T &item) {
    std::unique_lock lock(mutex_);
    if (deque_.empty() || deque_has_stopped_)
      return false;
    item = deque_.back();
    deque_.pop_back();
    return true;
  }

  void push_front(T item) {
    std::unique_lock lock(mutex_);
    deque_.push_front(item);
    lock.unlock();
    cond_.notify_one();
  }

  void push_front(std::vector<T> items) {
    std::unique_lock lock(mutex_);
    for (auto it = items.rbegin(); it != items.rend(); ++it)
      deque_.push_front(*it);
    lock.unlock();
    cond_.notify_all();
  }

  void push_back(T item) {
    std::unique_lock lock(mutex_);
    deque_.push_back(item);
    lock.unlock();
    cond_.notify_one();
  }

  void push_back(std::vector<T> items) {
    std::unique_lock lock(mutex_);
    for (auto item : items)
      deque_.push_back(item);
    lock.unlock();
    cond_.notify_all();
  }

  [[nodiscard]] bool empty() {
    std::unique_lock lock(mutex_);
    return deque_.empty();
  }

  [[nodiscard]] std::size_t size() {
    std::unique_lock lock(mutex_);
    return deque_.size();
  }

  void stop() {
    std::unique_lock lock(mutex_);
    deque_has_stopped_ = true;
    lock.unlock();
    cond_.notify_all();
  }

 private:
  std::deque<T> deque_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool deque_has_stopped_;
};

struct queue_stopped : public std::runtime_error {
  queue_stopped() : std::runtime_error("The queue has stopped") {}
};

template<typename T>
struct queue {

  queue() : queue_has_stopped_{false} {}

  queue(const queue &) = delete;            // disable copying
  queue &operator=(const queue &) = delete; // disable assignment

  [[nodiscard]] T wait_and_pop() {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&] { return !queue_.empty() || queue_has_stopped_; });
    if (queue_has_stopped_)
      throw queue_stopped();
    auto val = queue_.front();
    queue_.pop();
    lock.unlock();
    return val;
  }

  void wait_and_pop(T &item) {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&] { return !queue_.empty() || queue_has_stopped_; });
    if (queue_has_stopped_)
      throw queue_stopped();
    item = queue_.front();
    queue_.pop();
  }

  bool try_pop(T &item) {
    std::unique_lock lock(mutex_);
    if (queue_.empty() || queue_has_stopped_)
      return false;
    item = queue_.front();
    queue_.pop();
    return true;
  }

  void push(T item) {
    std::unique_lock lock(mutex_);
    queue_.push(item);
    lock.unlock();
    cond_.notify_one();
  }

  void push(std::vector<T> items) {
    std::unique_lock lock(mutex_);
    for (auto item : items)
      queue_.push(item);
    lock.unlock();
    cond_.notify_all();
  }

  [[nodiscard]] bool empty() {
    std::unique_lock lock(mutex_);
    return queue_.empty();
  }

  [[nodiscard]] std::size_t size() {
    std::unique_lock lock(mutex_);
    return queue_.size();
  }

  void stop() {
    std::unique_lock lock(mutex_);
    queue_has_stopped_ = true;
    lock.unlock();
    cond_.notify_all();
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool queue_has_stopped_;
};

} // concurrency

} // util

#endif //MOONSHINE_SHARED_UTIL_CONCURRENCY_H_
