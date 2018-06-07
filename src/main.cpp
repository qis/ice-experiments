#include <ice/async.hpp>
#include <ice/context.hpp>

int main(int argc, char* argv[])
{
  ice::context context;
  auto coro = [&]() -> ice::sync<int> {
    co_await ice::schedule(context);
    context.stop();
    co_return 0;
  }();
  context.run();
  return coro.get();
}
