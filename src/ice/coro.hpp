#pragma once
#include <ice/error.hpp>
#include <experimental/coroutine>

#if (defined(_MSC_VER) && _HAS_EXCEPTIONS) || !defined(_MSC_VER) && __cpp_exceptions
#include <exception>
#endif

namespace ice {

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

#if (defined(_MSC_VER) && _HAS_EXCEPTIONS) || !defined(_MSC_VER)
    void unhandled_exception() const noexcept
    {
#if (defined(_MSC_VER) && _HAS_EXCEPTIONS) || !defined(_MSC_VER) && __cpp_exceptions
      try {
        std::rethrow_exception(std::current_exception());
      }
      catch (const std::exception& e) {
        ice::err("unhandled exception: %s", e.what());
      }
      catch (...) {
#endif
        ice::err("unhandled exception");
#if (defined(_MSC_VER) && _HAS_EXCEPTIONS) || !defined(_MSC_VER) && __cpp_exceptions
      }
#endif
    }
#endif
  };
};

}  // namespace ice
