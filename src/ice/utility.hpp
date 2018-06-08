#pragma once
#include <ice/config.hpp>
#include <ice/handle.hpp>
#include <thread>
#include <type_traits>
#include <utility>
#include <cassert>

#if ICE_OS_WIN32
#include <windows.h>
#else
#include <pthread.h>
#if ICE_OS_FREEBSD
#include <pthread_np.h>
#endif
#endif

namespace ice {

#if ICE_OS_FREEBSD
using cpu_set_t = cpuset_t;
#endif

template <typename T>
class thread_local_storage {
public:
  using value_type = std::remove_cv_t<T>*;
#if ICE_OS_WIN32
  struct close_type {
    void operator()(DWORD handle) noexcept
    {
      ::TlsFree(handle);
    }
  };
  using handle_type = ice::handle<DWORD, 0, close_type>;
#else
  struct close_type {
    void operator()(pthread_key_t handle) noexcept
    {
      ::pthread_key_delete(handle);
    }
  };
  using handle_type = ice::handle<pthread_key_t, -1, close_type>;
#endif

  class lock {
  public:
    using handle_type = typename thread_local_storage::handle_type::view;

    lock(handle_type handle, value_type value) noexcept : handle_(handle)
    {
#if ICE_OS_WIN32
      [[maybe_unused]] const auto rc = ::TlsSetValue(handle_, value);
      assert(rc);
#else
      [[maybe_unused]] const auto rc = ::pthread_setspecific(handle_, value);
      assert(rc == 0);
#endif
    }

    lock(lock&& other) noexcept = default;
    lock& operator=(lock&& other) noexcept = default;

    ~lock()
    {
      if (handle_) {
#if ICE_OS_WIN32
        [[maybe_unused]] const auto rc = ::TlsSetValue(handle_, nullptr);
        assert(rc);
#else
        [[maybe_unused]] const auto rc = ::pthread_setspecific(handle_, nullptr);
        assert(rc == 0);
#endif
      }
    }

  private:
    handle_type handle_;
  };

  thread_local_storage() noexcept
  {
#if ICE_OS_WIN32
    handle_.reset(::TlsAlloc());
#else
    [[maybe_unused]] const auto rc = ::pthread_key_create(&handle_.value(), nullptr);
    assert(rc == 0);
#endif
    assert(handle_);
  }

  lock set(const value_type value) noexcept
  {
    return { handle_, value };
  }

  value_type get() noexcept
  {
#if ICE_OS_WIN32
    return reinterpret_cast<value_type>(::TlsGetValue(handle_));
#else
    return reinterpret_cast<value_type>(::pthread_getspecific(handle_));
#endif
  }

  const value_type get() const noexcept
  {
#if ICE_OS_WIN32
    return reinterpret_cast<const value_type>(::TlsGetValue(handle_));
#else
    return reinterpret_cast<const value_type>(::pthread_getspecific(handle_));
#endif
  }

private:
  handle_type handle_;
};

inline ice::error_code set_thread_affinity(std::size_t index) noexcept
{
  assert(index < std::thread::hardware_concurrency());
#if ICE_NO_DEBUG
#if ICE_OS_WIN32
  if (!::SetThreadAffinityMask(::GetCurrentThread(), DWORD_PTR(1) << index)) {
    return ::GetLastError();
  }
#else
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(static_cast<int>(index), &cpuset);
  if (const auto rc = ::pthread_setaffinity_np(::pthread_self(), sizeof(cpuset), &cpuset)) {
    return rc;
  }
#endif
#endif
  return {};
}

inline ice::error_code set_thread_affinity(std::thread& thread, std::size_t index) noexcept
{
  assert(index < std::thread::hardware_concurrency());
#if ICE_NO_DEBUG
#if ICE_OS_WIN32
  if (!::SetThreadAffinityMask(thread.native_handle(), DWORD_PTR(1) << index)) {
    return ::GetLastError();
  }
#else
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(static_cast<int>(index), &cpuset);
  if (const auto rc = ::pthread_setaffinity_np(thread.native_handle(), sizeof(cpuset), &cpuset)) {
    return rc;
  }
#endif
#endif
  return {};
}

template <typename Handler>
class scope_exit {
public:
  explicit scope_exit(Handler handler) noexcept : handler_(std::move(handler)) {}

  scope_exit(scope_exit&& other) noexcept :
    handler_(std::move(other.handler_)), invoke_(std::exchange(other.invoke_, false))
  {}

  scope_exit(const scope_exit& other) = delete;
  scope_exit& operator=(const scope_exit& other) = delete;

  ~scope_exit() noexcept(ICE_NO_EXCEPTIONS || noexcept(handler_()))
  {
    if (invoke_) {
      handler_();
    }
  }

private:
  Handler handler_;
  bool invoke_ = true;
};

template <typename Handler>
auto on_scope_exit(Handler&& handler) noexcept
{
  return ice::scope_exit<Handler>{ std::forward<Handler>(handler) };
}

}  // namespace ice
