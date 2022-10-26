#ifndef ASYNCQUEUE_ASYNCQUEUE_H
#define ASYNCQUEUE_ASYNCQUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace AsyncQueue {
    /**
     * @brief Asynchronous queue that allows for multiple input and output threads
     *
     * @tparam T The type stored in the queue
     */
    template <typename T> class AsyncQueue {
    public:
        /// @brief Get the mutex protecting the queue
        std::mutex &mutex() const { return m_mutex; }
        /// @brief Get the condition variable attached to the queue
        std::condition_variable &cv() { return m_cv; }
        /// @brief Acquire a lock on the queue
        std::unique_lock<std::mutex> lock() const { return std::unique_lock<std::mutex>(m_mutex); }
        /**
         * @brief Push a value to the queue
         *
         * The mutex is locked while the value is added and the condition variable is notified
         *
         * @param value The value to add
         */
        void push(const T &value);

        /**
         * @brief Push a value to the queue
         *
         * This version signals that the lock has already been acquired. The lock provided *must*
         * be for the correct mutex and locked.
         *
         * @param value The value to add
         * @param lock Should be an already acquired lock to the correct mutex
         */
        void push(const T &value, const std::unique_lock<std::mutex> &lock);

        /**
         * @brief Push a value to the queue
         *
         * The mutex is locked while the value is added and the condition variable is notified.
         * This version moves the provided value
         *
         * @param value
         */
        void push(T &&value);

        /**
         * @brief Push a value to the queue
         *
         *
         * @param value The value to add
         * @param lock Should be an already acquired lock to the correct mutex
         *
         * @param value
         */
        void push(T &&value, const std::unique_lock<std::mutex> &lock);

        /**
         * @brief Extract the first element of the queue
         *
         * The element is removed from the queue and returned. If the queue is empty the optional
         * will not be filled. Thread safe
         */
        std::optional<T> extract();

        /**
         * @brief Extract the first element of the queue
         *
         * The element is removed from the queue and returned. If the queue is empty the optional
         * will not be filled. The lock provided *must* be for the correct mutex and locked.
         *
         * @param lock Should be an already acquired lock to the correct mutex
         */
        std::optional<T> extract(const std::unique_lock<std::mutex> &lock);

    private:
        std::queue<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
    };
} // namespace AsyncQueue
#include "AsyncQueue/AsyncQueue.icc"
#endif //> !ASYNCQUEUE_ASYNCQUEUE_H