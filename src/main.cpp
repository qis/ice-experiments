#include <ice/coro.hpp>
#include <experimental/coroutine>

template <typename T>
class result {};

//result<void> test() noexcept
//{
//  co_return;
//}

int main(int argc, char* argv[])
{
  []() -> ice::task {
    co_await test();
    co_return;
  }();
}
