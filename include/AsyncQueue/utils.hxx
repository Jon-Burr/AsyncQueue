#ifndef ASYNCQUEUE_UTILS_HXX
#define ASYNCQUEUE_UTILS_HXX

#include <future>

namespace AsyncQueue {
    /// @brief Check if a future is ready
    /// @tparam T The return value of the future
    /// @param f The future
    template <typename T> bool isFutureReady(const std::future<T> &f);

    /// @brief Check if a future is ready
    /// @tparam T The return value of the future
    /// @param f The future
    template <typename T> bool isFutureReady(const std::shared_future<T> &f);

} // namespace AsyncQueue

#include "AsyncQueue/utils.ixx"

#endif //> !ASYNCQUEUE_UTILS_HXX