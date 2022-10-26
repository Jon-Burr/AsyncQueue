#include <functional>
#include <iostream>
#include <utility>

template <typename T> void foo(T value) { std::cout << value << std::endl; }

template <typename F, typename T> void bar(F &&f, T &&x) {
    std::invoke(std::forward<F>(f), std::forward<T>(x));
}

int main() {
    foo(7);

    void (*g)(int) = foo;
    g(3);
    bar(foo<int>, 2);

    void (*f)(void (*&&)(int), int &&) = bar;
    f(foo, 1);
    return 0;
}