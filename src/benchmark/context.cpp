#include <ice/context.hpp>
#include <ice/coro.hpp>
#include <benchmark/benchmark.h>
#include <thread>

static void context_schedule(benchmark::State& state) noexcept
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  [&](ice::context& c0, ice::context& c1) -> ice::task {
    bool first = true;
    for (auto _ : state) {
      if (first) {
        co_await ice::schedule(c0);
      } else {
        co_await ice::schedule(c1);
      }
      first = !first;
    }
    c0.stop();
    c1.stop();
  }(c0, c1);
  t0.join();
  t1.join();
}
BENCHMARK(context_schedule)->Threads(1);
