#include <ice/context.hpp>
#include <ice/coro.hpp>
#include <gtest/gtest.h>
#include <thread>

TEST(context, schedule)
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  [&](ice::context& c0, ice::context& c1) -> ice::task {
    co_await ice::schedule(c0, true);
    EXPECT_EQ(std::this_thread::get_id(), t0.get_id());
    co_await ice::schedule(c1, true);
    EXPECT_EQ(std::this_thread::get_id(), t1.get_id());
    co_await ice::schedule(c1);
    EXPECT_EQ(std::this_thread::get_id(), t1.get_id());
    co_await ice::schedule(c0);
    EXPECT_EQ(std::this_thread::get_id(), t0.get_id());
    c0.stop();
    c1.stop();
  }(c0, c1);
  t0.join();
  t1.join();
}
