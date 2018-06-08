#include <ice/async.hpp>
#include <ice/context.hpp>
#include <ice/utility.hpp>
#include <benchmark/benchmark.h>
#include <thread>

// ------------------------------------------------------------------------------------
// Benchmark                                             Time           CPU Iterations
// ------------------------------------------------------------------------------------
//
// Windows 10 1803 64-bit, Intel Core i7-2600 @ 3.4 GHz
// context_verify/iterations:10000000/threads:1          5 ns          5 ns   10000000
// context_append/iterations:10000000/threads:1         51 ns         52 ns   10000000
// context_switch/iterations:10000000/threads:1        104 ns         44 ns   10000000
// context_always/iterations:10000000/threads:1         69 ns         11 ns   10000000
//
// Windows 10 1803 64-bit WSL, Intel Core i7-2600 @ 3.4 GHz
// context_verify/iterations:10000000/threads:1          3 ns          3 ns   10000000
// context_append/iterations:10000000/threads:1         47 ns         47 ns   10000000
// context_switch/iterations:10000000/threads:1        110 ns         59 ns   10000000
// context_always/iterations:10000000/threads:1        152 ns        100 ns   10000000
//
// FreeBSD 12.0 64-bit, Intel Xeon E5-2660 v4 @ 2.0 GHz
// context_verify/iterations:10000000/threads:1          5 ns          5 ns   10000000
// context_append/iterations:10000000/threads:1         80 ns         80 ns   10000000
// context_switch/iterations:10000000/threads:1        190 ns        105 ns   10000000
// context_always/iterations:10000000/threads:1        270 ns        185 ns   10000000
//

constexpr std::size_t iterations = 10000000;

// Switches to the current context.
static void context_verify(benchmark::State& state) noexcept
{
  ice::context c0;
  auto co = [&]() -> ice::sync<void> {
    for (auto _ : state) {
      co_await ice::schedule(c0);
    }
    c0.stop();
  }();
  if (const auto ec = ice::set_thread_affinity(0)) {
    state.SkipWithError(ec.message().data());
  }
  c0.run();
  co.get();
}
BENCHMARK(context_verify)->Threads(1)->Iterations(iterations);

// Switches to the current context (suspends execution).
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
BENCHMARK(context_append)->Threads(1)->Iterations(iterations);

// Switches to the first context.
// Switches to the first context.
// Switches to the second context.
static void context_switch(benchmark::State& state) noexcept
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  if (const auto ec = ice::set_thread_affinity(t0, 0)) {
    state.SkipWithError(ec.message().data());
  }
  auto t1 = std::thread([&]() { c1.run(); });
  if (const auto ec = ice::set_thread_affinity(t1, 1)) {
    state.SkipWithError(ec.message().data());
  }
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
BENCHMARK(context_switch)->Threads(1)->Iterations(iterations);

// Switches to the first context.
// Switches to the second context.
static void context_always(benchmark::State& state) noexcept
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  if (const auto ec = ice::set_thread_affinity(t0, 0)) {
    state.SkipWithError(ec.message().data());
  }
  auto t1 = std::thread([&]() { c1.run(); });
  if (const auto ec = ice::set_thread_affinity(t1, 1)) {
    state.SkipWithError(ec.message().data());
  }
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
BENCHMARK(context_always)->Threads(1)->Iterations(iterations);
