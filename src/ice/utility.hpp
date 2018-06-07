#pragma once
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <type_traits>
#include <utility>
#include <cassert>

namespace ice {

template <typename T>
class thread_local_storage {
public:
  using value_type = std::remove_cv_t<T>*;
#ifdef WIN32
  using handle_type = DWORD;
  static constexpr handle_type invalid_handle_value = 0;
#else
  using handle_type = pthread_key_t;
  static constexpr handle_type invalid_handle_value = static_cast<handle_type>(-1);
#endif

  class lock {
  public:
    lock(handle_type handle, value_type value) noexcept : handle_(handle)
    {
#ifdef WIN32
      [[maybe_unused]] const auto rc = ::TlsSetValue(handle_, value);
      assert(rc);
#else
      [[maybe_unused]] const auto rc = ::pthread_setspecific(handle_, value);
      assert(rc == 0);
#endif
    }

    lock(lock&& other) : handle_(other.handle_)
    {
      other.handle_ = invalid_handle_value;
    }

    lock(const lock& other) = delete;

    lock& operator=(lock&& other)
    {
      lock{ std::move(other) }.swap(*this);
      return *this;
    }

    lock& operator=(const lock& other) = delete;

    ~lock()
    {
      if (handle_ != invalid_handle_value) {
#ifdef WIN32
        [[maybe_unused]] const auto rc = ::TlsSetValue(handle_, nullptr);
        assert(rc);
#else
        [[maybe_unused]] const auto rc = ::pthread_setspecific(handle_, nullptr);
        assert(rc == 0);
#endif
      }
    }

    constexpr void swap(lock& other) noexcept
    {
      const auto handle = other.handle_;
      other.handle_ = handle_;
      handle_ = handle;
    }

  private:
    handle_type handle_ = 0;
  };

  thread_local_storage() noexcept
  {
#ifdef WIN32
    handle_ = ::TlsAlloc();
#else
    [[maybe_unused]] const auto rc = ::pthread_key_create(&handle_, nullptr);
    assert(rc == 0);
#endif
    assert(handle_ != invalid_handle_value);
  }

  thread_local_storage(thread_local_storage&& other) : handle_(other.handle_)
  {
    other.handle_ = invalid_handle_value;
  }

  thread_local_storage(const thread_local_storage& other) = delete;

  thread_local_storage& operator=(thread_local_storage&& other)
  {
    thread_local_storage{ std::move(other) }.swap(*this);
    return *this;
  }

  thread_local_storage& operator=(const thread_local_storage& other) = delete;

  ~thread_local_storage()
  {
    if (handle_ != invalid_handle_value) {
#ifdef WIN32
      [[maybe_unused]] const auto rc = ::TlsFree(handle_);
      assert(rc);
#else
      [[maybe_unused]] const auto rc = ::pthread_key_delete(handle_);
      assert(rc == 0);
#endif
    }
  }

  constexpr void swap(thread_local_storage& other) noexcept
  {
    const auto handle = other.handle_;
    other.handle_ = handle_;
    handle_ = handle;
  }

  lock set(const value_type value) noexcept
  {
    return { handle_, value };
  }

  value_type get() noexcept
  {
#ifdef WIN32
    return reinterpret_cast<value_type>(::TlsGetValue(handle_));
#else
    return reinterpret_cast<value_type>(::pthread_getspecific(handle_));
#endif
  }

  const value_type get() const noexcept
  {
#ifdef WIN32
    return reinterpret_cast<const value_type>(::TlsGetValue(handle_));
#else
    return reinterpret_cast<const value_type>(::pthread_getspecific(handle_));
#endif
  }

private:
  handle_type handle_ = invalid_handle_value;
};

}  // namespace ice
