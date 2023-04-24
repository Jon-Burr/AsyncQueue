#ifndef ASYNCQUEUE_SIGNAL_HXX
#define ASYNCQUEUE_SIGNAL_HXX

#include <condition_variable>
#include <mutex>
#include <type_traits>

namespace AsyncQueue {
    /// @brief Helper class to pass around a bound CV+mutex pair
    /// @tparam Mutex The type of mutex used
    /// @tparam Lock The type of lock used
    class Signal {
    public:
        using lock_t = std::unique_lock<std::mutex>;
        Signal() = default;
        Signal(const Signal &) = delete;
        Signal(Signal &&) = delete;

        std::condition_variable &cv() { return m_cv; }
        const std::condition_variable &cv() const { return m_cv; }

        void notify_all() noexcept { cv().notify_all(); }
        void notify_one() noexcept { cv().notify_one(); }

        lock_t lock() { return lock_t(m_mutex); }

        void wait() {
            lock_t l = lock();
            cv().wait(l);
        }
        template <typename Rep, typename Period>
        std::cv_status wait_for(const std::chrono::duration<Rep, Period> &duration) {
            return cv().wait_for(lock(), duration);
        }
        template <typename Clock, typename Duration>
        std::cv_status wait_until(const std::chrono::time_point<Clock, Duration> &timeout) {
            return cv().wait_until(lock(), timeout);
        }

    private:
        std::condition_variable m_cv;
        std::mutex m_mutex;
    }; //> end class Signal
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_SIGNAL_HXX