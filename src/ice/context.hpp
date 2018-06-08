#pragma once
#include <ice/config.hpp>
#include <ice/utility.hpp>
#include <atomic>
#include <condition_variable>
#include <experimental/coroutine>
#include <mutex>

namespace ice {

class context {
public:
  class event {
  public:
    event() noexcept = default;

    // clang-format off
  #ifdef __INTELLISENSE__
    event(const event& other);
    event& operator=(const event& other);
  #else
    event(const event& other) = delete;
    event& operator=(const event& other) = delete;
  #endif
    // clang-format on

    virtual ~event() = default;

    void resume() noexcept
    {
      awaiter_.resume();
    }

  protected:
    std::experimental::coroutine_handle<> awaiter_;

  private:
    friend class context;
    std::atomic<event*> next_ = nullptr;
  };

  context() = default;

  context(const context& other) = delete;
  context& operator=(const context& other) = delete;

  void run() noexcept
  {
    const auto index = index_.set(this);
    std::unique_lock<std::mutex> lock{ mutex_ };
    lock.unlock();
    while (true) {
      lock.lock();
      auto head = head_.exchange(nullptr, std::memory_order_acquire);
      while (!head) {
        if (stop_.load(std::memory_order_acquire)) {
          lock.unlock();
          return;
        }
        cv_.wait(lock, []() { return true; });
        head = head_.exchange(nullptr, std::memory_order_acquire);
      }
      lock.unlock();
      while (head) {
        auto next = head->next_.load(std::memory_order_relaxed);
        head->resume();
        head = next;
      }
    }
  }

  bool is_current() const noexcept
  {
    return index_.get() ? true : false;
  }

  void stop() noexcept
  {
    stop_.store(true, std::memory_order_release);
    cv_.notify_all();
  }

  void schedule(event* ev) noexcept
  {
    auto head = head_.load(std::memory_order_acquire);
    do {
      ev->next_.store(head, std::memory_order_relaxed);
    } while (!head_.compare_exchange_weak(head, ev, std::memory_order_release, std::memory_order_acquire));
    cv_.notify_one();
  }

private:
  std::atomic_bool stop_ = false;
  std::atomic<event*> head_ = nullptr;
  ice::thread_local_storage<context> index_;
  std::condition_variable cv_;
  std::mutex mutex_;
};

class schedule final : public ice::context::event {
public:
  schedule(context& context, bool post = false) noexcept : context_(context), ready_(!post && context.is_current()) {}

  constexpr bool await_ready() const noexcept
  {
    return ready_;
  }

  void await_suspend(std::experimental::coroutine_handle<> awaiter) noexcept
  {
    awaiter_ = awaiter;
    context_.schedule(this);
  }

  constexpr void await_resume() const noexcept {}

private:
  context& context_;
  const bool ready_ = true;
};

}  // namespace ice
