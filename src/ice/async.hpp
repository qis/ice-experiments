#pragma once
#include <ice/config.hpp>
#include <atomic>
#include <condition_variable>
#include <experimental/coroutine>
#include <mutex>
#include <optional>

#if ICE_EXCEPTIONS
#include <ice/error.hpp>
#include <exception>
#endif

namespace ice {

template <typename T>
struct sync {
  struct state {
    std::optional<T> value;
    std::atomic_bool ready = false;
    std::condition_variable cv;
    std::mutex mutex;
#if ICE_EXCEPTIONS
    std::exception_ptr exception;
#endif
  };

  struct promise_type {
    sync get_return_object() noexcept
    {
      return { this };
    }

    constexpr auto initial_suspend() const noexcept
    {
      return std::experimental::suspend_never{};
    }

    constexpr auto final_suspend() const noexcept
    {
      return std::experimental::suspend_never{};
    }

    template <typename... Args>
    void return_value(Args&&... args) noexcept
    {
      state_->value.emplace(std::forward<Args>(args)...);
      state_->ready.store(true, std::memory_order_release);
      state_->cv.notify_one();
    }

#if ICE_EXCEPTIONS || defined(__clang__)
    void unhandled_exception() noexcept
    {
#if ICE_EXCEPTIONS
      state_->exception = std::current_exception();
      state_->ready.store(true, std::memory_order_release);
      state_->cv.notify_one();
#endif
    }
#endif

    state* state_ = nullptr;
  };

  sync(promise_type* promise) noexcept : state_(std::make_unique<state>())
  {
    promise->state_ = state_.get();
  }

  T& get() & noexcept(!ICE_EXCEPTIONS)
  {
    std::unique_lock<std::mutex> lock(state_->mutex);
    state_->cv.wait(lock, [this]() { return state_->ready.load(std::memory_order_acquire); });
#if ICE_EXCEPTIONS
    if (state_->exception) {
      std::rethrow_exception(state_->exception);
    }
#endif
    return *state_->value;
  }

  T&& get() && noexcept(!ICE_EXCEPTIONS)
  {
    std::unique_lock<std::mutex> lock(state_->mutex);
    state_->cv.wait(lock, [this]() { return state_->ready.load(std::memory_order_acquire); });
#if ICE_EXCEPTIONS
    if (state_->exception) {
      std::rethrow_exception(state_->exception);
    }
#endif
    return std::move(*state_->value);
  }

private:
  std::unique_ptr<state> state_;
};

template <>
struct sync<void> {
  struct state {
    std::atomic_bool ready = false;
    std::condition_variable cv;
    std::mutex mutex;
#if ICE_EXCEPTIONS
    std::exception_ptr exception;
#endif
  };

  struct promise_type {
    sync get_return_object() noexcept
    {
      return { this };
    }

    constexpr auto initial_suspend() const noexcept
    {
      return std::experimental::suspend_never{};
    }

    constexpr auto final_suspend() const noexcept
    {
      return std::experimental::suspend_never{};
    }

    void return_void() noexcept
    {
      state_->ready.store(true, std::memory_order_release);
      state_->cv.notify_one();
    }

#if ICE_EXCEPTIONS || defined(__clang__)
    void unhandled_exception() noexcept
    {
#if ICE_EXCEPTIONS
      state_->exception = std::current_exception();
      state_->ready.store(true, std::memory_order_release);
      state_->cv.notify_one();
#endif
    }
#endif

    state* state_ = nullptr;
  };

  sync(promise_type* promise) noexcept : state_(std::make_unique<state>())
  {
    promise->state_ = state_.get();
  }

  void get() noexcept(!ICE_EXCEPTIONS)
  {
    std::unique_lock<std::mutex> lock(state_->mutex);
    state_->cv.wait(lock, [this]() { return state_->ready.load(std::memory_order_acquire); });
#if ICE_EXCEPTIONS
    if (state_->exception) {
      std::rethrow_exception(state_->exception);
    }
#endif
  }

private:
  std::unique_ptr<state> state_;
};

struct task {
  struct promise_type {
    constexpr task get_return_object() const noexcept
    {
      return {};
    }

    constexpr auto initial_suspend() const noexcept
    {
      return std::experimental::suspend_never{};
    }

    constexpr auto final_suspend() const noexcept
    {
      return std::experimental::suspend_never{};
    }

    constexpr void return_void() const noexcept {}

#if ICE_EXCEPTIONS || defined(__clang__)
    void unhandled_exception() const noexcept
    {
#if ICE_EXCEPTIONS
      try {
        throw;
      }
      catch (const std::exception& e) {
        ice::err("unhandled exception: %s", e.what());
      }
      catch (...) {
        ice::err("unhandled exception");
      }
#endif
    }
#endif
  };
};

}  // namespace ice
