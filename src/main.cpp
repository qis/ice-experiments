#include <atomic>
#include <experimental/coroutine>
#include <mutex>
#include <optional>

template <typename T>
struct sync {
  struct state {
    std::optional<T> value;
    std::atomic_bool ready = false;
    std::condition_variable cv;
    std::mutex mutex;
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
    //constexpr void return_void() const noexcept {}

#if (defined(_MSC_VER) && _HAS_EXCEPTIONS) || !defined(_MSC_VER)
    constexpr void unhandled_exception() const noexcept {}  // TODO
#endif

    state* state_ = nullptr;
  };

  sync(promise_type* promise) noexcept : state_(std::make_unique<state>())
  {
    promise->state_ = state_.get();
  }

  T& get() & noexcept
  {
    std::unique_lock<std::mutex> lock(state_->mutex);
    state_->cv.wait(lock, [this]() { return state_->ready.load(std::memory_order_acquire); });
    return *state_->value;
  }

  T&& get() && noexcept
  {
    std::unique_lock<std::mutex> lock(state_->mutex);
    state_->cv.wait(lock, [this]() { return state_->ready.load(std::memory_order_acquire); });
    return std::move(*state_->value);
  }

private:
  std::unique_ptr<state> state_;
};

#include <ice/context.hpp>

int main(int argc, char* argv[])
{
  ice::context context;
  auto coro = [&]() -> sync<int> {
    co_await ice::schedule(context);
    context.stop();
    co_return 1;
  }();
  context.run();
  return coro.get();
}
