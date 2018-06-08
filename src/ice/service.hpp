#pragma once
#include <ice/error.hpp>
#include <ice/utility.hpp>
#include <system_error>
#include <vector>
#include <cassert>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else
#ifdef __linux__
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif
#ifdef __FreeBSD__
#include <sys/event.h>
#endif
#include <unistd.h>
#endif

namespace ice {

class service {
public:
#ifdef WIN32
  using handle_type = HANDLE;
  constexpr static handle_type invalid_handle_value = nullptr;

  class event : public OVERLAPPED {
  public:
    event() noexcept : OVERLAPPED({}) {}

    // clang-format off
  #ifdef __INTELLISENSE__
    event(event&& other) {}
    event(const event& other) {}
    event& operator=(event&& other) { return *this; }
    event& operator=(const event& other) { return *this; }
  #else
    event(event&& other) = delete;
    event(const event& other) = delete;
    event& operator=(event&& other) = delete;
    event& operator=(const event& other) = delete;
  #endif
    // clang-format on

    virtual ~event() = default;

    bool await_suspend(std::experimental::coroutine_handle<> awaiter) noexcept
    {
      awaiter_ = awaiter;
      return suspend();
    }

    void await_resume() noexcept
    {
      if (resume() || !suspend()) {
        awaiter_.resume();
      }
    }

    virtual bool suspend() noexcept = 0;
    virtual bool resume() noexcept = 0;

  protected:
    std::experimental::coroutine_handle<> awaiter_;
  };

#else
  using handle_type = int;
  constexpr static handle_type invalid_handle_value = -1;
#endif

  service() noexcept
  {
#ifdef WIN32
    struct wsa {
      wsa()
      {
        WSADATA wsadata = {};
        [[maybe_unused]] const auto rc = ::WSAStartup(MAKEWORD(2, 2), &wsadata);
        assert(rc == 0);
      }
    };
    static const wsa wsa;
    handle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
    assert(handle_ != invalid_handle_value);
#endif
#ifdef __linux__
    handle_ = ::epoll_create1(0);
    assert(handle_ != invalid_handle_value);
    events_ = ::eventfd(0, EFD_NONBLOCK);
    assert(events_ != invalid_handle_value);
    epoll_event nev = { EPOLLONESHOT, {} };
    [[maybe_unused]] const auto rc = ::epoll_ctl(handle_, EPOLL_CTL_ADD, events_, &nev);
    assert(rc == 0);
#endif
#ifdef __FreeBSD__
    handle_ = ::kqueue();
    assert(handle_ != invalid_handle_value);
    struct kevent nev;
    EV_SET(&nev, 0, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    [[maybe_unused]] const auto rc = ::kevent(handle_, &nev, 1, nullptr, 0, nullptr);
    assert(rc == 1);
#endif
  }

  ~service()
  {
    if (handle_ != invalid_handle_value) {
#ifdef WIN32
      [[maybe_unused]] const auto rc = ::CloseHandle(handle_);
      assert(rc);
#else
      [[maybe_unused]] const auto rc = ::close(handle_);
      assert(rc == 0);
#endif
    }
  }

  ice::error_code run(std::size_t event_buffer_size = 256)
  {
#ifdef WIN32
    using data_type = OVERLAPPED_ENTRY;
    using size_type = ULONG;
#endif
#ifdef __linux__
    using data_type = epoll_event;
    using size_type = int;
#endif
#ifdef __FreeBSD__
    using data_type = struct kevent;
    using size_type = int;
#endif

    ice::error_code ec;
    std::vector<data_type> events;
    events.resize(event_buffer_size);

    const auto events_data = events.data();
    const auto events_size = static_cast<size_type>(events.size());
    const auto index = index_.set(this);

    while (true) {
#ifdef WIN32
      size_type count = 0;
      if (!::GetQueuedCompletionStatusEx(handle_, events_data, events_size, &count, INFINITE, FALSE)) {
        if (const auto rc = ::GetLastError(); rc != ERROR_ABANDONED_WAIT_0) {
          ec = rc;
        }
        break;
      }
#else
#ifdef __linux__
      const auto count = ::epoll_wait(handle_, events_data, events_size, -1);
#endif
#ifdef __FreeBSD__
      const auto count = ::kevent(handle_, nullptr, 0, events_data, events_size, nullptr);
#endif
      if (count < 0 && errno != EINTR) {
        ec = errno;
        break;
      }
#endif
      bool interrupted = false;
      for (size_type i = 0; i < count; i++) {
        auto& entry = events_data[i];
#ifdef WIN32
        if (const auto ev = static_cast<event*>(entry.lpOverlapped)) {
          ev->await_resume();
          continue;
        }
#endif
#ifdef __linux__
        if (const auto ev = reinterpret_cast<event*>(entry.data.ptr)) {
          ev->await_resume();
          continue;
        }
#endif
#ifdef __FreeBSD__
        if (const auto ev = reinterpret_cast<event*>(entry.udata)) {
          ev->await_resume();
          continue;
        }
#endif
        interrupted = true;
      }
      if (interrupted) {
        break;
      }
    }
    return ec;
  }

  void stop() noexcept
  {
#ifdef WIN32
    ::PostQueuedCompletionStatus(handle_, 0, 0, nullptr);
#endif
#ifdef __linux__
    epoll_event nev{ EPOLLOUT | EPOLLONESHOT, {} };
    ::epoll_ctl(handle_, EPOLL_CTL_MOD, events_, &nev);
#endif
#ifdef __FreeBSD__
    struct kevent nev {};
    EV_SET(&nev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);
    ::kevent(handle_, &nev, 1, nullptr, 0, nullptr);
#endif
  }

private:
  handle_type handle_ = invalid_handle_value;
#ifdef __linux__
  handle_type events_ = invalid_handle_value;
#endif
  ice::thread_local_storage<service> index_;
};

}  // namespace ice
