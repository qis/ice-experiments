#include "error.hpp"
#include <ice/config.hpp>
#include <algorithm>
#include <cctype>
#include <cstdio>

#if 0
#include <cstdarg>
#include <mutex>
#include <ctime>
#endif

#if ICE_OS_WIN32
#include <wchar.h>
#include <windows.h>
#endif

namespace ice {
namespace {

class native_category : public std::error_category {
public:
  const char* name() const noexcept override
  {
    return "native";
  }

  std::string message(int code) const override
  {
#if ICE_OS_WIN32
    std::wstring wstr;
    constexpr DWORD type = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    constexpr DWORD lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);  // MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    const auto id = static_cast<DWORD>(code);
    LPWSTR buffer = nullptr;
    if (const auto size = FormatMessageW(type, nullptr, id, lang, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr)) {
      wstr.assign(buffer, wcsnlen_s(buffer, static_cast<size_t>(size)));
      LocalFree(reinterpret_cast<HLOCAL>(buffer));
    }
    if (!wstr.empty()) {
      const auto src_data = wstr.data();
      const auto src_size = static_cast<int>(wstr.size());
      auto size = WideCharToMultiByte(CP_UTF8, 0, src_data, src_size, nullptr, 0, nullptr, nullptr) + 1;
      if (size > 0) {
        std::string str(static_cast<std::size_t>(size) + 1, '\0');
        size = WideCharToMultiByte(CP_UTF8, 0, src_data, src_size, str.data(), size, nullptr, nullptr);
        if (size > 0) {
          str.resize(strnlen_s(str.data(), static_cast<size_t>(size)));
          return format(str);
        }
      }
    }
    return "unknown error";
#else
    return format(std::system_category().message(code));
#endif
  }

  static std::string format(std::string str)
  {
    if (const auto pos = str.find_first_not_of("\r\n "); pos != std::string::npos) {
      str.erase(0, pos);
    }
    if (const auto pos = str.find('.'); pos != std::string::npos) {
      str.erase(pos);
    }
    if (const auto pos = str.find_last_not_of("\r\n "); pos != std::string::npos) {
      str.erase(pos + 1);
    }
    auto ascii = true;
    std::transform(str.begin(), str.end(), str.begin(), [&ascii](char c) {
      if (ascii && c >= ' ' && c <= '~') {
        return static_cast<char>(std::tolower(c));
      }
      ascii = false;
      return c;
    });
    return str;
  }
};

class system_category : public std::error_category {
public:
  const char* name() const noexcept override
  {
    return "system";
  }

  std::string message(int code) const override
  {
    const auto value = static_cast<std::errc>(code);
    if (value == std::errc::address_family_not_supported) {
      return "address family not supported";
    }
    if (value == std::errc::address_in_use) {
      return "address in use";
    }
    if (value == std::errc::address_not_available) {
      return "address not available";
    }
    if (value == std::errc::already_connected) {
      return "already connected";
    }
    if (value == std::errc::argument_list_too_long) {
      return "argument list too long";
    }
    if (value == std::errc::argument_out_of_domain) {
      return "argument out of domain";
    }
    if (value == std::errc::bad_address) {
      return "bad address";
    }
    if (value == std::errc::bad_file_descriptor) {
      return "bad file descriptor";
    }
    if (value == std::errc::bad_message) {
      return "bad message";
    }
    if (value == std::errc::broken_pipe) {
      return "broken pipe";
    }
    if (value == std::errc::connection_aborted) {
      return "connection aborted";
    }
    if (value == std::errc::connection_already_in_progress) {
      return "connection already in progress";
    }
    if (value == std::errc::connection_refused) {
      return "connection refused";
    }
    if (value == std::errc::connection_reset) {
      return "connection reset";
    }
    if (value == std::errc::cross_device_link) {
      return "cross device link";
    }
    if (value == std::errc::destination_address_required) {
      return "destination address required";
    }
    if (value == std::errc::device_or_resource_busy) {
      return "device or resource busy";
    }
    if (value == std::errc::directory_not_empty) {
      return "directory not empty";
    }
    if (value == std::errc::executable_format_error) {
      return "executable format error";
    }
    if (value == std::errc::file_exists) {
      return "file exists";
    }
    if (value == std::errc::file_too_large) {
      return "file too large";
    }
    if (value == std::errc::filename_too_long) {
      return "filename too long";
    }
    if (value == std::errc::function_not_supported) {
      return "function not supported";
    }
    if (value == std::errc::host_unreachable) {
      return "host unreachable";
    }
    if (value == std::errc::identifier_removed) {
      return "identifier removed";
    }
    if (value == std::errc::illegal_byte_sequence) {
      return "illegal byte sequence";
    }
    if (value == std::errc::inappropriate_io_control_operation) {
      return "inappropriate io control operation";
    }
    if (value == std::errc::interrupted) {
      return "interrupted";
    }
    if (value == std::errc::invalid_argument) {
      return "invalid argument";
    }
    if (value == std::errc::invalid_seek) {
      return "invalid seek";
    }
    if (value == std::errc::io_error) {
      return "io error";
    }
    if (value == std::errc::is_a_directory) {
      return "is a directory";
    }
    if (value == std::errc::message_size) {
      return "message size";
    }
    if (value == std::errc::network_down) {
      return "network down";
    }
    if (value == std::errc::network_reset) {
      return "network reset";
    }
    if (value == std::errc::network_unreachable) {
      return "network unreachable";
    }
    if (value == std::errc::no_buffer_space) {
      return "no buffer space";
    }
    if (value == std::errc::no_child_process) {
      return "no child process";
    }
    if (value == std::errc::no_link) {
      return "no link";
    }
    if (value == std::errc::no_lock_available) {
      return "no lock available";
    }
    if (value == std::errc::no_message_available) {
      return "no message available";
    }
    if (value == std::errc::no_message) {
      return "no message";
    }
    if (value == std::errc::no_protocol_option) {
      return "no protocol option";
    }
    if (value == std::errc::no_space_on_device) {
      return "no space on device";
    }
    if (value == std::errc::no_stream_resources) {
      return "no stream resources";
    }
    if (value == std::errc::no_such_device_or_address) {
      return "no such device or address";
    }
    if (value == std::errc::no_such_device) {
      return "no such device";
    }
    if (value == std::errc::no_such_file_or_directory) {
      return "no such file or directory";
    }
    if (value == std::errc::no_such_process) {
      return "no such process";
    }
    if (value == std::errc::not_a_directory) {
      return "not a directory";
    }
    if (value == std::errc::not_a_socket) {
      return "not a socket";
    }
    if (value == std::errc::not_a_stream) {
      return "not a stream";
    }
    if (value == std::errc::not_connected) {
      return "not connected";
    }
    if (value == std::errc::not_enough_memory) {
      return "not enough memory";
    }
    if (value == std::errc::not_supported) {
      return "not supported";
    }
    if (value == std::errc::operation_canceled) {
      return "operation canceled";
    }
    if (value == std::errc::operation_in_progress) {
      return "operation in progress";
    }
    if (value == std::errc::operation_not_permitted) {
      return "operation not permitted";
    }
    if (value == std::errc::operation_not_supported) {
      return "operation not supported";
    }
    if (value == std::errc::operation_would_block) {
      return "operation would block";
    }
    if (value == std::errc::owner_dead) {
      return "owner dead";
    }
    if (value == std::errc::permission_denied) {
      return "permission denied";
    }
    if (value == std::errc::protocol_error) {
      return "protocol error";
    }
    if (value == std::errc::protocol_not_supported) {
      return "protocol not supported";
    }
    if (value == std::errc::read_only_file_system) {
      return "read only file system";
    }
    if (value == std::errc::resource_deadlock_would_occur) {
      return "resource deadlock would occur";
    }
    if (value == std::errc::resource_unavailable_try_again) {
      return "resource unavailable try again";
    }
    if (value == std::errc::result_out_of_range) {
      return "result out of range";
    }
    if (value == std::errc::state_not_recoverable) {
      return "state not recoverable";
    }
    if (value == std::errc::stream_timeout) {
      return "stream timeout";
    }
    if (value == std::errc::text_file_busy) {
      return "text file busy";
    }
    if (value == std::errc::timed_out) {
      return "timed out";
    }
    if (value == std::errc::too_many_files_open_in_system) {
      return "too many files open in system";
    }
    if (value == std::errc::too_many_files_open) {
      return "too many files open";
    }
    if (value == std::errc::too_many_links) {
      return "too many links";
    }
    if (value == std::errc::too_many_symbolic_link_levels) {
      return "too many symbolic link levels";
    }
    if (value == std::errc::value_too_large) {
      return "value too large";
    }
    if (value == std::errc::wrong_protocol_type) {
      return "wrong protocol type";
    }
    return "unknown error";
  }
};

class domain_category : public std::error_category {
public:
  const char* name() const noexcept override
  {
    return "domain";
  }

  std::string message(int code) const override
  {
    switch (static_cast<ice::errc>(code)) {
    case ice::errc::eof: return "end of file";
    case ice::errc::version: return "version mismatch";
    }
    return "unknown error";
  }
};

const native_category g_native_category;
const system_category g_system_category;
const domain_category g_domain_category;

#if 0

using clock = std::chrono::system_clock;

std::mutex& mutex()
{
  static std::mutex mutex;
  return mutex;
}

void print(FILE* out, clock::time_point tp, const char* category) noexcept
{
  const auto tt = std::chrono::system_clock::to_time_t(tp);
  tm tm = {};
#if ICE_OS_WIN32
  localtime_s(&tm, &tt);
#else
  localtime_r(&tt, &tm);
#endif
  [[maybe_unused]] const auto Y = tm.tm_year + 1900;
  [[maybe_unused]] const auto M = tm.tm_mon + 1;
  [[maybe_unused]] const auto D = tm.tm_mday;
  const auto h = tm.tm_hour;
  const auto m = tm.tm_min;
  const auto s = tm.tm_sec;
#ifdef ICE_LOG_MILLISECONDS
  const auto tse = tp.time_since_epoch();
  const auto sse = std::chrono::duration_cast<std::chrono::seconds>(tse);
  const auto mse = std::chrono::duration_cast<std::chrono::milliseconds>(tse) - sse;
  const auto ms = static_cast<int>(mse.count());
#ifdef ICE_LOG_DATE
  std::fprintf(out, "%04d-%02d-%02d %02d:%02d:%02d.%03d [", Y, M, D, h, m, s, ms);
#else
  std::fprintf(out, "%02d:%02d:%02d.%03d [", h, m, s, ms);
#endif
#else
#ifdef ICE_LOG_DATE
  std::fprintf(out, "%04d-%02d-%02d %02d:%02d:%02d [", Y, M, D, h, m, s);
#else
  std::fprintf(out, "%02d:%02d:%02d [", h, m, s);
#endif
#endif
#if ICE_OS_WIN32
  HANDLE handle = nullptr;
  CONSOLE_SCREEN_BUFFER_INFO info;
  WORD reset = 0;
  if (out == stderr) {
    handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle && ::GetConsoleScreenBufferInfo(handle, &info)) {
      reset = info.wAttributes;
      info.wAttributes &= ~(info.wAttributes & 0x0F);
      info.wAttributes |= static_cast<WORD>(FOREGROUND_RED | FOREGROUND_INTENSITY);
      if (!::SetConsoleTextAttribute(handle, info.wAttributes)) {
        handle = nullptr;
      }
    } else {
      handle = nullptr;
    }
  }
  std::fputs(category, out);
  if (handle) {
    ::SetConsoleTextAttribute(handle, reset);
  }
#else
  if (out == stderr) {
    std::fprintf(out, "\033[31m%s\033[00m", category);
  } else {
    std::fputs(category, out);
  }
#endif
  std::fputs("] ", out);
}

#endif

}  // namespace

const std::error_category& native_category()
{
  return g_native_category;
}

const std::error_category& system_category()
{
  return g_system_category;
}

const std::error_category& domain_category()
{
  return g_domain_category;
}

#if 0

void log(const char* format, ...) noexcept
{
  std::lock_guard<std::mutex> lock(mutex());
  print(stdout, clock::now(), "custom");
  va_list args;
  va_start(args, format);
  std::vfprintf(stdout, format, args);
  va_end(args);
  std::fputc('\n', stdout);
}

void err(const char* format, ...) noexcept
{
  std::lock_guard<std::mutex> lock(mutex());
  print(stderr, clock::now(), "custom");
  va_list args;
  va_start(args, format);
  std::vfprintf(stderr, format, args);
  va_end(args);
  std::fputc('\n', stderr);
}

void log(ice::error_code ec, const char* format, ...) noexcept
{
  std::lock_guard<std::mutex> lock(mutex());
  print(stdout, clock::now(), ec.category().name());
  va_list args;
  va_start(args, format);
  std::vfprintf(stdout, format, args);
  va_end(args);
  std::fprintf(stdout, ": %s (%d)\n", ec.message().data(), ec.value());
}

void err(ice::error_code ec, const char* format, ...) noexcept
{
  std::lock_guard<std::mutex> lock(mutex());
  print(stderr, clock::now(), ec.category().name());
  va_list args;
  va_start(args, format);
  std::vfprintf(stderr, format, args);
  va_end(args);
  std::fprintf(stderr, ": %s (%d)\n", ec.message().data(), ec.value());
}

void log(ice::error_code ec) noexcept
{
  std::lock_guard<std::mutex> lock(mutex());
  print(stdout, clock::now(), ec.category().name());
  std::fprintf(stdout, "%s (%d)\n", ec.message().data(), ec.value());
}

void err(ice::error_code ec) noexcept
{
  std::lock_guard<std::mutex> lock(mutex());
  print(stderr, clock::now(), ec.category().name());
  std::fprintf(stderr, "%s (%d)\n", ec.message().data(), ec.value());
}

#endif

}  // namespace ice
