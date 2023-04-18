#ifndef ASYNCQUEUE_UTILS_HXX
#define ASYNCQUEUE_UTILS_HXX

#include <chrono>
#include <future>
#include <tuple>

namespace AsyncQueue {
    /// @brief Check if a future is ready
    /// @tparam T The return value of the future
    /// @param f The future
    template <typename T> bool isFutureReady(const std::future<T> &f);

    /// @brief Check if a future is ready
    /// @tparam T The return value of the future
    /// @param f The future
    template <typename T> bool isFutureReady(const std::shared_future<T> &f);

    namespace detail {
        /// @brief Type trait that gives whether or not a type is a std::chrono::duration
        /// @tparam T The type to be checked
        template <typename T> struct is_duration : public std::false_type {};
        template <typename Rep, typename Period>
        struct is_duration<std::chrono::duration<Rep, Period>> : public std::true_type {};

        /// @brief Holds whether or not the provided type is a std::chrono::duration
        template <typename T> static constexpr inline bool is_duration_v = is_duration<T>::value;
    } // namespace detail

} // namespace AsyncQueue

#include "AsyncQueue/utils.ixx"

#endif //> !ASYNCQUEUE_UTILS_HXX