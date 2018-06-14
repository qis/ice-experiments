#include <ice/error.hpp>
#include <ice/log.hpp>

int main(int argc, char* argv[])
{
  ice::error_code ec = std::errc::address_family_not_supported;
  ice::log("test: %d", 0);
  ice::err("test: %d", 1);
  ice::log(ec, "test: %d", 2);
  ice::err(ec, "test: %d", 3);
  ice::log(ec);
  ice::err(ec);
  //ice::context context;
  //auto coro = [&]() -> ice::sync<int> {
  //  context.stop();
  //  co_return 0;
  //}();
  //context.run();
  //return coro.get();
}
