#include <ice/async.hpp>
#include <ice/context.hpp>

int main(int argc, char* argv[])
{
  ice::context context;
  auto coro = [&]() -> ice::sync<int> {
    context.stop();
    co_return 0;
  }();
  context.run();
  return coro.get();
}
