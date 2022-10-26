#include <type_traits>
#include <utility>

#include <functional>
#include <future>
#include <iostream>
#include <string_view>

template <typename T> constexpr auto type_name() {
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}
template <typename F, typename... Args>
std::invoke_result_t<F, Args...> invoke(F &&f, Args &&... args) {
    static_assert(
            std::is_invocable_v<F, Args...>, "f must be callable with the supplied arguments");
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

template <typename F, typename... Args>
std::invoke_result_t<F, Args...> myinvoke(F &&f, Args &&... args) {
    return std::invoke(invoke<F, Args...>, std::forward<F>(f), std::forward<Args>(args)...);
}
template <typename F, typename... Args>
std::future<std::invoke_result_t<F, Args...>> myasync(F &&f, Args &&... args) {
    // std::invoke_result_t<F, Args...> (*g)(F &&, Args && ...) = std::invoke<F, Args...>;
    // int (*g)(int (*&&)(int, int), int &&, int &&) = std::invoke<int (*)(int, int), int, int>;
    int (*g)(int (*)(int, int), int &&, int &&) = std::invoke<F, Args...>;

    return std::async(g, std::forward<F>(f), std::forward<Args>(args)...);
    // return std::async(invoke<F &&, Args &&...>, std::forward<F>(f), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
std::future<T> myasync2(std::function<T(Args...)> f, Args &&... args) {
    T(*g)
    (std::function<T(Args...)> &&, Args && ...) = std::invoke<std::function<T(Args...)>, Args...>;
    return std::async(g, f, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
std::future<T> myasync3(std::function<T(Args...)> f, Args &&... args) {
    // T(*g)
    // (std::function<T(Args...)> &&, Args && ...) = std::invoke<std::function<T(Args...)>,
    // Args...>;
    return std::async(
            std::invoke<std::function<T(Args...)>, Args...>, f, std::forward<Args>(args)...);
}

template <typename F, typename... Args>
std::future<std::invoke_result_t<F, Args...>> myasync4(F &&f, Args &&... args) {
    using T = std::invoke_result_t<F, Args...>;
    return std::async(
            std::invoke<std::function<T(Args...)>, Args...>, std::function<T(Args...)>(f),
            std::forward<Args>(args)...);
}

template <typename F, typename... Args> std::string_view invoke_type_name(F &&f, Args &&... args) {
    return type_name<std::invoke<F, Args...>>();
}

int foo(int a, int b) { return a + b; }

int bar(int (*f)(int, int), int a, int b) { return f(a, b); }

int main() {
    std::cout << myinvoke(foo, 1, 2) << std::endl;
    std::cout << std::async(foo, 1, 2).get() << std::endl;
    std::cout << bar(foo, 1, 2) << std::endl;
    std::cout << std::async(bar, foo, 1, 2).get() << std::endl;
    int (*f)(int (*&&)(int, int), int &&, int &&) = std::invoke<int (*)(int, int), int, int>;

    std::cout << f(foo, 2, 3) << std::endl;
    std::cout << std::async(f, foo, 6, 1).get() << std::endl;

    int (*g)(std::function<int(int, int)> &&, int &&, int &&) =
            std::invoke<std::function<int(int, int)>, int, int>;
    std::cout << g(foo, 1, 0) << std::endl;
    std::cout << std::async(g, foo, 9, 3).get() << std::endl;
    std::cout << myasync2(std::function<int(int, int)>(foo), 2, 3).get() << std::endl;
    std::cout << myasync3(std::function<int(int, int)>(foo), 2, 3).get() << std::endl;
    std::cout << myasync4(foo, 6, 3).get() << std::endl;

    // std::cout << myasync(&foo, 1, 2).get() << std::endl;
    // std::cout << myasync(foo, 1, 2).get() << std::endl;
    return 0;
}