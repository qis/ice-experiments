#pragma once
#include <ice/error.hpp>
#include <fmt/format.h>
#include <chrono>

namespace ice {

class log {
public:
  using clock = std::chrono::system_clock;

  enum class level {
    emergency = 0,
    alert = 1,
    critical = 2,
    error = 3,
    warning = 4,
    notice = 5,
    info = 6,
    debug = 7,
  };

  enum class color : unsigned {
    none = 0x0,
    grey = 0x1,
    red = 0x2,
    green = 0x3,
    yellow = 0x4,
    blue = 0x5,
    magenta = 0x6,
    cyan = 0x7,
    white = 0x8,
  };

  enum class style : unsigned {
    none = 0x0,
    bold = 0x1 << 8,
    dark = 0x2 << 8,
    underline = 0x3 << 8,
    blink = 0x4 << 8,
    reverse = 0x5 << 8,
    concealed = 0x6 << 8,
  };

  class format {
  public:
    constexpr format() noexcept = default;
    constexpr explicit format(unsigned format) noexcept : format_(format) {}
    constexpr format(log::color color) noexcept : format(static_cast<unsigned>(color)) {}
    constexpr format(log::style style) noexcept : format(static_cast<unsigned>(style)) {}

    constexpr explicit operator bool() const noexcept
    {
      return format_ ? true : false;
    }

    constexpr format& operator|(log::color color) noexcept
    {
      format_ = static_cast<unsigned>(style()) | static_cast<unsigned>(color);
      return *this;
    }

    constexpr format& operator|(log::style style) noexcept
    {
      format_ = static_cast<unsigned>(color()) | static_cast<unsigned>(style);
      return *this;
    }

    constexpr log::color color() const noexcept
    {
      return static_cast<log::color>(static_cast<unsigned>(format_) & 0x00FF);
    }

    constexpr log::style style() const noexcept
    {
      return static_cast<log::style>(static_cast<unsigned>(format_) & 0xFF00);
    }

  private:
    unsigned format_ = 0;
  };

  constexpr static inline format operator|(color color, style style) noexcept
  {
    return format{ static_cast<unsigned>(color) | static_cast<unsigned>(style) };
  }

  constexpr static inline format operator|(style style, color color) noexcept
  {
    return format{ static_cast<unsigned>(style) | static_cast<unsigned>(color) };
  }

  // ice::log("details: {}", ec);
  template <typename... Args>
  log(const char* message, Args&&... args) noexcept
  {
    const auto tp = clock::now();
    queue(tp, level::info, format{}, fmt::format(message, std::forward<Args>(args)...));
  }

  // ice::log(ice::log::level::warning, "details: {}", ec);
  template <typename... Args>
  log(level level, const char* message, Args&&... args) noexcept
  {
    const auto tp = clock::now();
    queue(tp, level, format{}, fmt::format(message, std::forward<Args>(args)...));
  }

  // ice::log(ice::log::color::red | ice::log::style::bold, "details: {}", ec);
  template <typename... Args>
  log(format format, const char* message, Args&&... args) noexcept
  {
    const auto tp = clock::now();
    queue(tp, level::info, format, fmt::format(message, std::forward<Args>(args)...));
  }

  // ice::log(ice::log::level::warning, ice::log::color::red | ice::log::style::bold, "details: {}", ec);
  template <typename... Args>
  log(level level, format format, const char* message, Args&&... args) noexcept
  {
    const auto tp = clock::now();
    queue(tp, level, format, fmt::format(message, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void emergency(const char* message, Args&&... args) noexcept
  {
    log{ level::emergency, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void emergency(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::emergency, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void alert(const char* message, Args&&... args) noexcept
  {
    log{ level::alert, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void alert(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::alert, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void critical(const char* message, Args&&... args) noexcept
  {
    log{ level::critical, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void critical(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::critical, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void error(const char* message, Args&&... args) noexcept
  {
    log{ level::error, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void error(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::error, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void warning(const char* message, Args&&... args) noexcept
  {
    log{ level::warning, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void warning(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::warning, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void notice(const char* message, Args&&... args) noexcept
  {
    log{ level::notice, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void notice(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::notice, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void info(const char* message, Args&&... args) noexcept
  {
    log{ level::info, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void info(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::info, format, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void debug(const char* message, Args&&... args) noexcept
  {
    log{ level::debug, message, std::forward<Args>(args)... };
  }

  template <typename... Args>
  static void debug(format format, const char* message, Args&&... args) noexcept
  {
    log{ level::debug, format, message, std::forward<Args>(args)... };
  }

  static void queue(clock::time_point tp, level level, format format, std::string message) noexcept;
};

}  // namespace ice
