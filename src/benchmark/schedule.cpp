#include <benchmark/benchmark.h>
#include <ice/coro.hpp>
#include <ice/context.hpp>
#include <thread>

static ice::task coro(ice::context& c0, ice::context& c1, benchmark::State& state) noexcept {
  bool first = true;
  for (auto _ : state) {
    if (first) {
      co_await ice::schedule{ c0 };
    } else {
      co_await ice::schedule{ c1 };
    }
    first = !first;
  }
  c0.stop();
  c1.stop();
  co_return;
}

static void schedule(benchmark::State& state) noexcept {
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  coro(c0, c1, state);
  t0.join();
  t1.join();
}

BENCHMARK(schedule)->Threads(1);
