#include <ice/error.hpp>
#include <experimental/coroutine>

struct task {
  struct promise_type {
    promise_type() noexcept {
      //ice::log("task::promise_type [+]");
    }

    ~promise_type() {
      //ice::log("task::promise_type [-]");
    }

    task get_return_object() const noexcept {
      //ice::log("task::promise_type::get_return_object");
      return {};
    }

    auto initial_suspend() const noexcept {
      //ice::log("task::promise_type::initial_suspend");
      return std::experimental::suspend_never{};
    }

    auto final_suspend() const noexcept {
      //ice::log("task::promise_type::final_suspend");
      return std::experimental::suspend_never{};
    }

    void return_void() const noexcept {
      //ice::log("task::promise_type::return_void");
    }

#if defined(__clang__) || (defined(_MSC_VER) && _HAS_EXCEPTIONS)
    void unhandled_exception() const noexcept {
      ice::err("task::promise_type::unhandled_exception");
    }
#endif
  };

  task() noexcept {
    //ice::log("task [+]");
  }

  ~task() {
    //ice::log("task [-]");
  }
};

//template <typename T>
//class result {
//
//};
//
//result<void> test() noexcept {
//  co_return;
//}
//
//task coro() noexcept {
//  co_await test();
//  co_return;
//}

int main(int argc, char* argv[]) {
}

// msvc
// main [+]
// test(context) [+]
// task::promise_type [+]
// task::promise_type::initial_suspend
// task::promise_type::get_return_object
// task [+]
// task [-]
// test(context) [-]
// task::promise_type::return_void
// task::promise_type::final_suspend
// task::promise_type [-]
// main [-]

// clang
// main [+]
// test(context) [+]
// task::promise_type [+]
// task::promise_type::get_return_object
// task [+]
// task::promise_type::initial_suspend
// task [-]
// test(context) [-]
// task::promise_type::return_void
// task::promise_type::final_suspend
// task::promise_type [-]
// main [-]
