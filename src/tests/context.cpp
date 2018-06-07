#include <ice/async.hpp>
#include <ice/context.hpp>
#include <gtest/gtest.h>
#include <thread>

TEST(context, schedule)
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });
  auto co = [&]() -> ice::sync<void> {
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
  }();
  t0.join();
  t1.join();
  co.get();
}
