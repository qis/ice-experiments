#pragma once
#include <ice/error.hpp>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else
#ifdef __linux__
#include <sys/epoll.h>
#endif
#ifdef __FreeBSD__
#include <sys/event.h>
#endif
#include <unistd.h>
#endif
#include <system_error>
#include <cassert>

namespace ice {
namespace detail {

#ifdef WIN32

struct wsa {
  wsa()
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

#endif

}  // namespace detail

class service {
public:
  service() noexcept = default;

  ice::error_code create() noexcept
  {
#ifdef WIN32
    static const detail::wsa wsa;
    if (wsa.ec) {
      return wsa.ec;
    }
#endif
    return {};
  }
};

}  // namespace ice
