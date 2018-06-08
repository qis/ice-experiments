#pragma once
#include <ice/config.hpp>
#include <ice/handle.hpp>
#include <experimental/coroutine>
#include <utility>
#include <vector>

#if ICE_OS_WIN32
#include <windows.h>
#include <winsock2.h>
#else
#if ICE_OS_LINUX
#include <sys/epoll.h>
#include <sys/eventfd.h>
#elif ICE_OS_FREEBSD
#include <sys/event.h>
#endif
#include <unistd.h>
#endif

namespace ice {

class service {
public:
#if ICE_OS_WIN32
  struct close_type {
    void operator()(HANDLE handle) noexcept
    {
      ::CloseHandle(handle);
    }
  };

  using handle_type = ice::handle<HANDLE, nullptr, close_type>;

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
  struct close_type {
    void operator()(HANDLE handle) noexcept
    {
      ::close(handle);
    }
  };

  using handle_type = ice::handle<int, -1, close_type>;
#endif

  service() noexcept = default;

  ice::error_code create() noexcept
  {
#if ICE_OS_WIN32
    struct wsa {
      wsa() noexcept
      {
        WSADATA wsadata = {};
        if (const auto rc = ::WSAStartup(MAKEWORD(2, 2), &wsadata)) {
          ec = rc;
        }
        const auto major = LOBYTE(wsadata.wVersion);
        const auto minor = HIBYTE(wsadata.wVersion);
        if (major < 2 || (major == 2 && minor < 2)) {
          ec = ice::errc::version;
        }
      }
      ice::error_code ec;
    };
    static const wsa wsa;
    if (wsa.ec) {
      return wsa.ec;
    }
    handle_type handle(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0));
    if (!handle) {
      return ::GetLastError();
    }
#elif ICE_OS_LINUX
    handle_type handle(::epoll_create1(0));
    if (!handle) {
      return errno;
    }
    handle_type events(::eventfd(0, EFD_NONBLOCK));
    if (!events) {
      return errno;
    }
    epoll_event nev = { EPOLLONESHOT, {} };
    if (::epoll_ctl(handle, EPOLL_CTL_ADD, events, &nev) < 0) {
      return errno;
    }
    events_ = std::move(events);
#elif ICE_OS_FREEBSD
    handle_type handle(::kqueue());
    if (!handle) {
      return errno;
    }
    struct kevent nev = {};
    EV_SET(&nev, 0, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    if (::kevent(handle, &nev, 1, nullptr, 0, nullptr) < 0) {
      return errno;
    }
#endif
    handle_ = std::move(handle);
    return {};
  }

  ice::error_code run(std::size_t event_buffer_size = 128)
  {
#if ICE_OS_WIN32
    using data_type = OVERLAPPED_ENTRY;
    using size_type = ULONG;
#elif ICE_OS_LINUX
    using data_type = epoll_event;
    using size_type = int;
#elif ICE_OS_FREEBSD
    using data_type = struct kevent;
    using size_type = int;
#endif

    ice::error_code ec;
    std::vector<data_type> events;
    events.resize(event_buffer_size);

    const auto events_data = events.data();
    const auto events_size = static_cast<size_type>(events.size());

    while (true) {
#if ICE_OS_WIN32
      size_type count = 0;
      if (!::GetQueuedCompletionStatusEx(handle_, events_data, events_size, &count, INFINITE, FALSE)) {
        if (const auto rc = ::GetLastError(); rc != ERROR_ABANDONED_WAIT_0) {
          ec = rc;
        }
        break;
      }
#elif ICE_OS_LINUX
      const auto count = ::epoll_wait(handle_, events_data, events_size, -1);
      if (count < 0 && errno != EINTR) {
        ec = errno;
        break;
      }
#elif ICE_OS_FREEBSD
      const auto count = ::kevent(handle_, nullptr, 0, events_data, events_size, nullptr);
      if (count < 0 && errno != EINTR) {
        ec = errno;
        break;
      }
#endif
      bool interrupted = false;
      for (size_type i = 0; i < count; i++) {
        auto& entry = events_data[i];
#if ICE_OS_WIN32
        if (const auto ev = static_cast<event*>(entry.lpOverlapped)) {
          ev->await_resume();
          continue;
        }
#elif ICE_OS_LINUX
        if (const auto ev = reinterpret_cast<event*>(entry.data.ptr)) {
          ev->await_resume();
          continue;
        }
#elif ICE_OS_FREEBSD
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
#if ICE_OS_WIN32
    ::PostQueuedCompletionStatus(handle_, 0, 0, nullptr);
#elif ICE_OS_LINUX
    epoll_event nev{ EPOLLOUT | EPOLLONESHOT, {} };
    ::epoll_ctl(handle_, EPOLL_CTL_MOD, events_, &nev);
#elif ICE_OS_FREEBSD
    struct kevent nev {};
    EV_SET(&nev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);
    ::kevent(handle_, &nev, 1, nullptr, 0, nullptr);
#endif
  }

private:
  handle_type handle_;
#if ICE_OS_LINUX
  handle_type events_;
#endif
};

}  // namespace ice
