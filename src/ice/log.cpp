#include "log.hpp"
#include <ice/config.hpp>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <cstdio>
#include <ctime>

namespace ice {
namespace {

class logger {
public:
  struct entry {
    log::clock::time_point tp;
    log::level level = log::level::info;
    log::format format;
    std::string message;
  };

  static inline bool print_date = true;
  static inline bool print_time = true;
  static inline bool print_milliseconds = true;
  static inline bool print_level = true;

  static void print(FILE* out, log::clock::time_point tp) noexcept
  {
    const auto tt = std::chrono::system_clock::to_time_t(tp);
    tm tm = {};
#if ICE_OS_WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    const auto Y = tm.tm_year + 1900;
    const auto M = tm.tm_mon + 1;
    const auto D = tm.tm_mday;
    const auto h = tm.tm_hour;
    const auto m = tm.tm_min;
    const auto s = tm.tm_sec;
    auto ms = 0;
    if (print_milliseconds) {
      const auto tse = tp.time_since_epoch();
      const auto sse = std::chrono::duration_cast<std::chrono::seconds>(tse);
      const auto mse = std::chrono::duration_cast<std::chrono::milliseconds>(tse) - sse;
      const auto ms = static_cast<int>(mse.count());
    }
    if (print_date) {
      if (print_time) {
        if (print_milliseconds) {
          std::fprintf(out, "%04d-%02d-%02d %02d:%02d:%02d.%03d ", Y, M, D, h, m, s, ms);
        } else {
          std::fprintf(out, "%04d-%02d-%02d %02d:%02d:%02d ", Y, M, D, h, m, s);
        }
      } else {
        std::fprintf(out, "%04d-%02d-%02d ", Y, M, D);
      }
    } else {
      if (print_milliseconds) {
        std::fprintf(out, "%02d:%02d:%02d.%03d ", h, m, s, ms);
      } else {
        std::fprintf(out, "%02d:%02d:%02d ", h, m, s);
      }
    }
  }

  static void print(FILE* out, log::level level) noexcept
  {
    switch (level) {
    case log::level::emergency: std::fputs("emergency", out); break;
    case log::level::alert:     std::fputs("alert    ", out); break;
    case log::level::critical:  std::fputs("critical ", out); break;
    case log::level::error:     std::fputs("error    ", out); break;
    case log::level::warning:   std::fputs("warning  ", out); break;
    case log::level::notice:    std::fputs("notice   ", out); break;
    case log::level::info:      std::fputs("info     ", out); break;
    case log::level::debug:     std::fputs("debug    ", out); break;
    }
  }

  static void print(const entry& entry) noexcept
  {
    auto out = static_cast<int>(entry.level) > static_cast<int>(log::level::error) ? stdout : stderr;
    if (print_date || print_time) {
      print(out, entry.tp);
    }
    if (print_level) {
      std::fputc('[', out);
      // TODO: Set color and style.
      print(out, entry.level);
      // TODO: Reset color and style.
      std::fputs("] ", out);
    }
    if (entry.format) {
      // TODO: Set color and style.
    }
    std::fputs(entry.message.data(), out);
    if (entry.format) {
      // TODO: Reset color and style.
    }
    std::fflush(out);
  }

  static void queue(entry entry) noexcept
  {
    static logger logger;
    std::lock_guard<std::mutex> lock(logger.mutex_);
    if (!logger.stop_.load(std::memory_order_acquire) && !logger.thread_) {
      logger.thread_ = std::make_unique<std::thread>([]() { logger.run(); });
    }
    logger.queue_.push_back(std::move(entry));
    logger.cv_.notify_one();
  }

private:
  logger() noexcept {}

  ~logger()
  {
    stop_.store(true, std::memory_order_release);
    cv_.notify_all();
    if (thread_ && thread_->joinable()) {
      thread_->join();
    }
  }

  void run() noexcept
  {
    while (!stop_.load(std::memory_order_acquire)) {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, []() { return true; });
      const auto queue = std::move(queue_);
      queue_.clear();
      lock.unlock();
      for (const auto& entry : queue) {
        print(entry);
      }
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& entry : queue_) {
      print(entry);
    }
  }

  std::mutex mutex_;
  std::deque<entry> queue_;
  std::condition_variable cv_;
  std::unique_ptr<std::thread> thread_;
  std::atomic_bool stop_ = false;
};

}  // namespace

void log::queue(clock::time_point tp, level level, format format, std::string message) noexcept
{
  logger::queue({ tp, level, format, std::move(message) });
}

}  // namespace ice
