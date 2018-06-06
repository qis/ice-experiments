# Notes
<https://lewissbaker.github.io/2017/09/25/coroutine-theory>

## MSVC Task Order
```
main [+]
test(context) [+]
task::promise_type [+]
task::promise_type::initial_suspend
task::promise_type::get_return_object
task [+]
task [-]
test(context) [-]
task::promise_type::return_void
task::promise_type::final_suspend
task::promise_type [-]
main [-]
```

## LLVM Task Order
```
main [+]
test(context) [+]
task::promise_type [+]
task::promise_type::get_return_object
task [+]
task::promise_type::initial_suspend
task [-]
test(context) [-]
task::promise_type::return_void
task::promise_type::final_suspend
task::promise_type [-]
main [-]
```
