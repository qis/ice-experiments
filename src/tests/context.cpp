#include <ice/async.hpp>
#include <ice/context.hpp>
#include <gtest/gtest.h>
#include <thread>

// Verifies that the schedule operation works.
TEST(context, schedule)
{
  ice::context c0;
  ice::context c1;
  auto t0 = std::thread([&]() { c0.run(); });
  auto t1 = std::thread([&]() { c1.run(); });

  auto co = [&]() -> ice::sync<void> {
    EXPECT_NE(std::this_thread::get_id(), t0.get_id());
    EXPECT_NE(std::this_thread::get_id(), t1.get_id());
    EXPECT_FALSE(c0.is_current());
    EXPECT_FALSE(c1.is_current());

    co_await ice::schedule(c0, true);
    EXPECT_EQ(std::this_thread::get_id(), t0.get_id());
    EXPECT_TRUE(c0.is_current());
    EXPECT_FALSE(c1.is_current());

    co_await ice::schedule(c1, true);
    EXPECT_EQ(std::this_thread::get_id(), t1.get_id());
    EXPECT_FALSE(c0.is_current());
    EXPECT_TRUE(c1.is_current());

    co_await ice::schedule(c1);
    EXPECT_EQ(std::this_thread::get_id(), t1.get_id());
    EXPECT_FALSE(c0.is_current());
    EXPECT_TRUE(c1.is_current());

    co_await ice::schedule(c0);
    EXPECT_EQ(std::this_thread::get_id(), t0.get_id());
    EXPECT_TRUE(c0.is_current());
    EXPECT_FALSE(c1.is_current());

    c0.stop();
    c1.stop();
  }();

  t0.join();
  t1.join();
  co.get();
}
