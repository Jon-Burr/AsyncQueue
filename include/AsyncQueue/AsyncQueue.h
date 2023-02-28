#ifndef ASYNCQUEUE_ASYNCQUEUE_H
#define ASYNCQUEUE_ASYNCQUEUE_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <queue>

#include "AsyncQueue/ThreadManager.h"

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
         * @param value The value to add
         * @param lock Should be an already acquired lock to the correct mutex
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

        /**
         * @brief Start looping a task that produces items and adds them to the queue
         * @tparam F The function producing the values. It *must* take a reference to this queue as
         *      its first argument.
         * @tparam Args Any other arguments that should be passed to the function
         * @param mgr The thread manager that controls the execution of this loop
         * @param producer Function that produces the value
         * @param args Any further arguments (beyond this queue) that will be provided to the
         *      function
         */
        template <typename F, typename... Args>
        std::future<TaskStatus> loopProducer(ThreadManager &mgr, F &&producer, Args &&...args);

        /**
         * @brief Start looping a task that produces items and adds them to the queue
         *
         * This version is for tasks that will wait for a notification from some condition variable
         * before they produce any output. This cv will be added to the list which should be
         * notified when the manager aborts the loop.
         *
         * @tparam F The function producing the values. It *must* take a reference to this queue as
         *      its first argument.
         * @tparam Args Any other arguments that should be passed to the function
         * @param mgr The thread manager that controls the execution of this loop
         * @param cv Condition variable which will be notified when the manager aborts the loop
         * @param producer Function that produces the value
         * @param args Any further arguments (beyond this queue) that will be provided to the
         *      function
         */
        template <typename F, typename... Args>
        std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, std::condition_variable &cv, F &&producer, Args &&...args);

        /**
         * @brief Start looping a task that produces items and adds them to the queue
         * @tparam Rep std::chrono::duration template parameter
         * @tparam Period std::chrono::duration template parameter
         * @tparam F The function producing the values. It *must* take a reference to this queue as
         *      its first argument.
         * @tparam Args Any other arguments that should be passed to the function
         * @param mgr The thread manager that controls the execution of this loop
         * @param producer Function that produces the value
         * @param heartbeat Time to wait between executions
         * @param args Any further arguments (beyond this queue) that will be provided to the
         *      function
         */
        template <typename Rep, typename Period, typename F, typename... Args>
        std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, const std::chrono::duration<Rep, Period> &heartbeat,
                F &&producer, Args &&...args);

        /**
         * @brief Start looping a task that consumes items from the list
         *
         * Multiple consumers can be registered with a queue but each item from the queue will go
         * to only *one* consumer. Each consuming thread will be woken whenever data is available
         * for it.
         *
         * @tparam F The function producing the values. It *must* take an entry from the list as
         *      its first argument.
         * @tparam Args Any other arguments that should be passed to the function
         * @param mgr The thread manager that controls the execution of this loop
         * @param consumer Function that consumes the value
         * @param args Any further arguments (beyond this queue) that will be provided to the
         *      function
         */
        template <typename F, typename... Args>
        std::future<TaskStatus> loopConsumer(ThreadManager &mgr, F &&consumer, Args &&...args);

        /**
         * @brief Start looping a task that produces items and adds them to the queue
         * @param mgr The thread manager that controls the execution of this loop
         * @param producer Function that produces the value
         */
        std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, std::function<TaskStatus(AsyncQueue<T> &)> producer);

        /**
         * @brief Start looping a task that produces items and adds them to the queue
         *
         * This version is for tasks that will wait for a notification from some condition variable
         * before they produce any output. This cv will be added to the list which should be
         * notified when the manager aborts the loop.
         *
         * @param mgr The thread manager that controls the execution of this loop
         * @param cv Condition variable which will be notified when the manager aborts the loop
         * @param producer Function that produces the value
         */
        std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, std::condition_variable &cv,
                std::function<TaskStatus(AsyncQueue<T> &)> producer);

        /**
         * @brief Start looping a task that consumes items from the list
         *
         * Multiple consumers can be registered with a queue but each item from the queue will go
         * to only *one* consumer. Each consuming thread will be woken whenever data is available
         * for it.
         *
         * @param mgr The thread manager that controls the execution of this loop
         * @param consumer Function that consumes the value
         */
        std::future<TaskStatus> loopConsumer(
                ThreadManager &mgr, std::function<TaskStatus(const T &)> consumer);

        /// @brief Internal function that does the actual consumer loop
        TaskStatus doLoopConsumer(std::function<TaskStatus(const T &)> consumer);

    private:
        std::queue<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
    };
} // namespace AsyncQueue
#include "AsyncQueue/AsyncQueue.icc"
#endif //> !ASYNCQUEUE_ASYNCQUEUE_H