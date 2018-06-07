#include <gtest/gtest.h>
#include <ice/coro.hpp>
#include <ice/context.hpp>
#include <thread>

TEST(context, schedule)
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  [](ice::context& c0, ice::context& c1) -> ice::task {
    co_await ice::schedule(c0, true);
    const auto ct0 = std::this_thread::get_id();
    co_await ice::schedule(c1, true);
    const auto ct1 = std::this_thread::get_id();
    EXPECT_NE(ct0, ct1);
    co_await ice::schedule(c1);
    EXPECT_EQ(std::this_thread::get_id(), ct1);
    co_await ice::schedule(c0);
    EXPECT_EQ(std::this_thread::get_id(), ct0);
    c0.stop();
    c1.stop();
  }(c0, c1);
  t0.join();
  t1.join();
}
