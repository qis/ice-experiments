#include <ice/async.hpp>
#include <ice/context.hpp>
#include <benchmark/benchmark.h>
#include <thread>

static void context_schedule(benchmark::State& state) noexcept
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  auto co = [&]() -> ice::sync<void> {
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
  }();
  t0.join();
  t1.join();
  co.get();
}
BENCHMARK(context_schedule)->Threads(1);
