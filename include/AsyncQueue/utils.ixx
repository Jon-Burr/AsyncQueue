#include <chrono>

namespace AsyncQueue {
    template <typename T> bool isFutureReady(const std::future<T> &f) {
        using namespace std::chrono_literals;
        return f.wait_for(0s) == std::future_status::ready;
    }

    template <typename T> bool isFutureReady(const std::shared_future<T> &f) {
        using namespace std::chrono_literals;
        return f.wait_for(0s) == std::future_status::ready;
    }
} // namespace AsyncQueue