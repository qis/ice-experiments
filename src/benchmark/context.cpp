#include <ice/async.hpp>
#include <ice/context.hpp>
#include <benchmark/benchmark.h>
#include <thread>

// ----------------------------------------------------------------
// Benchmark                         Time           CPU Iterations
// ----------------------------------------------------------------
// Windows 10 64-bit, Intel Core i7-2600 @ 3.4 GHz
// context_verify/threads:1          6 ns          6 ns  112000000
// context_append/threads:1         53 ns         53 ns   11200000
// context_switch/threads:1        171 ns        102 ns   20363636
// context_always/threads:1        205 ns         64 ns   10000000
//
// Windows 10 64-bit WSL, Intel Core i7-2600 @ 3.4 GHz
// context_verify/threads:1          3 ns          3 ns  213333333
// context_append/threads:1         48 ns         48 ns   14933333
// context_switch/threads:1        120 ns         98 ns  100000000
// context_always/threads:1        164 ns        142 ns  100000000

static void context_verify(benchmark::State& state) noexcept
{
  ice::context c0;
  auto co = [&]() -> ice::sync<void> {
    for (auto _ : state) {
      co_await ice::schedule(c0);
    }
    c0.stop();
  }();
  c0.run();
  co.get();
}
BENCHMARK(context_verify)->Threads(1);

static void context_append(benchmark::State& state) noexcept
{
  ice::context c0;
  auto co = [&]() -> ice::sync<void> {
    for (auto _ : state) {
      co_await ice::schedule(c0, true);
    }
    c0.stop();
  }();
  c0.run();
  co.get();
}
BENCHMARK(context_append)->Threads(1);

static void context_switch(benchmark::State& state) noexcept
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  auto co = [&]() -> ice::sync<void> {
    auto i = 0;
    for (auto _ : state) {
      switch (i % 3) {
      case 0: [[fallthrough]];
      case 1: co_await ice::schedule(c0); break;
      case 2: co_await ice::schedule(c1); break;
      }
      i++;
    }
    c0.stop();
    c1.stop();
  }();
  t0.join();
  t1.join();
  co.get();
}
BENCHMARK(context_switch)->Threads(1);

static void context_always(benchmark::State& state) noexcept
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  auto co = [&]() -> ice::sync<void> {
    auto i = true;
    for (auto _ : state) {
      if (i) {
        co_await ice::schedule(c0, true);
      } else {
        co_await ice::schedule(c1, true);
      }
      i = !i;
    }
    c0.stop();
    c1.stop();
  }();
  t0.join();
  t1.join();
  co.get();
}
BENCHMARK(context_always)->Threads(1);
