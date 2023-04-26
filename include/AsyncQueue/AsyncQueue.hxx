/**
 * @file AsyncQueue.hxx
 * @brief Asynchronous queue implementation
 */

#ifndef ASYNCQUEUE_ASYNCQUEUE_HXX
#define ASYNCQUEUE_ASYNCQUEUE_HXX

#include "AsyncQueue/Fwd.hxx"
#include "AsyncQueue/TaskStatus.hxx"
#include "AsyncQueue/ThreadManager.hxx"
#include "AsyncQueue/concepts.hxx"
#include "AsyncQueue/packManipulation.hxx"

#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <type_traits>
#include <utility>

namespace AsyncQueue {

    /// @brief Asynchronous queue implementation
    /// @tparam T The data type stored in the queue
    ///
    /// Multiple consumers and producers can be looped on the queue. Each element of the queue will
    /// be sent to a single producer. This will be determined by the condition variable's notify_one
    /// method and should be assumed to be non-deterministic.
    template <typename T> class AsyncQueue {
    public:
        using lock_t = std::unique_lock<std::mutex>;

        /// @brief Get the mutex protecting the queue
        std::mutex &mutex() const;
        /// @brief Acquire a lock on the queue
        lock_t lock() const;
        /// @brief Get the condition variable for the queue
        std::condition_variable &cv();

        /// @name Push methods
        /// The push methods take a const reference (copy) or an rvalue reference (move) to a value
        /// to add to the queue. For both version a lock can also be provided. In this case it
        /// *must* be the queue's own lock. If a lock isn't provided the @ref lock method will be
        /// used to supply one.
        /// @{
        void push(const T &value);
        void push(const T &value, const lock_t &lock);
        void push(T &&value);
        void push(T &&value, const lock_t &lock);
        /// @}

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
        std::optional<T> extract(const lock_t &lock);

        /// @brief Get the number of elements in the queue
        std::size_t size() const;

        /// @brief Get the number of elements in the queue
        std::size_t size(const lock_t &) const;

        /// @brief Is the queue empty?
        bool empty() const;

        /// @brief Is the queue empty?
        bool empty(const lock_t &) const;

        template <std::size_t I = 0, typename F, typename... Args>
        requires concepts::Producer<I, F, T, Args...> std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, F &&f, Args &&... args);

        template <std::size_t I = 0, concepts::Duration D, typename F, typename... Args>
        requires concepts::Producer<I, F, T, Args...> std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, const D &heartbeat, F &&f, Args &&... args);

        template <std::size_t I = 0, typename F, typename... Args>
        requires concepts::Producer<I, F, T, Args...> std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, std::condition_variable &cv, F &&f, Args &&... args);

        template <std::size_t I = 0, concepts::Duration D, typename F, typename... Args>
        requires concepts::Producer<I, F, T, Args...> std::future<TaskStatus> loopProducer(
                ThreadManager &mgr, std::condition_variable &cv, const D &heartbeat, F &&f,
                Args &&... args);

        template <std::size_t I = 0, typename F, typename... Args>
        requires concepts::Consumer<I, F, T, Args...> std::future<TaskStatus> loopConsumer(
                ThreadManager &mgr, F &&f, Args &&... args);

        template <std::size_t I = 0, concepts::Duration D, typename F, typename... Args>
        requires concepts::Consumer<I, F, T, Args...> std::future<TaskStatus> loopConsumer(
                ThreadManager &mgr, const D &heartbeat, F &&f, Args &&... args);

    private:
        template <std::size_t I, typename F, typename... Args>
        requires concepts::ProducerTask<I, F, T, Args...> TaskStatus
        doLoopProducer(F f, Args... args);

        template <std::size_t I, typename F, typename... Args>
        requires concepts::ProducerVoid<I, F, T, Args...> void doLoopProducer(F f, Args... args);

        template <std::size_t I, typename F, typename... Args>
        requires concepts::ConsumerTask<I, F, T, Args...> TaskStatus
        doLoopConsumer(F f, Args... args);

        template <std::size_t I, typename F, typename... Args>
        requires concepts::ConsumerVoid<I, F, T, Args...> void doLoopConsumer(F f, Args... args);

        std::queue<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
    }; //> end class AsyncQueue<T>
} // namespace AsyncQueue

#include "AsyncQueue/AsyncQueue.ixx"

#endif //> !ASYNCQUEUE_ASYNCQUEUE_HXX