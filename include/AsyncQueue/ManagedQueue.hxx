#ifndef ASYNCQUEUE_MANAGEDQUEUE_HXX
#define ASYNCQUEUE_MANAGEDQUEUE_HXX

#include "AsyncQueue/AsyncQueue.hxx"
#include "AsyncQueue/IConsumer.hxx"
#include <memory>

namespace AsyncQueue {
    /// @brief Async queue with its own associated stop source and dedicated consumer
    /// @tparam T The data type stored in the queue
    ///
    /// When the associated stop has been requested any remaining elements of the queue will be
    /// processed by the consumer, but no new elements will be accepted. The consumer pointer can be
    /// owned and managed by the queue, or it can merely hold an observing pointer.
    template <typename T> class ManagedQueue {
    public:
        using lock_t = std::unique_lock<std::mutex>;
#ifdef AsyncQueue_MULTITHREAD

        ManagedQueue(std::stop_source ss, std::unique_ptr<IConsumer<T>> consumer);
        ManagedQueue(std::stop_source ss, IConsumer<T> *consumer);
        template <std::derived_from<IConsumer<T>> Consumer>
            requires std::move_constructible<Consumer>
        ManagedQueue(std::stop_source ss, Consumer &&consumer);
#endif
        ManagedQueue(std::unique_ptr<IConsumer<T>> consumer);
        template <std::derived_from<IConsumer<T>> Consumer>
            requires std::move_constructible<Consumer>
        ManagedQueue(Consumer &&consumer);
        ManagedQueue(IConsumer<T> *consumer);
        ~ManagedQueue();

        /// @brief Get the mutex protecting the queue
        std::mutex &mutex() const { return m_queue.mutex(); }
        /// @brief Acquire a lock on the queue
        lock_t lock() const { return m_queue.lock(); }
        /// @brief Get the condition variable for the queue
        std::condition_variable_any &cv() { return m_queue.cv(); }

#ifdef AsyncQueue_MULTITHREAD
        /// @brief Get the stop source
        std::stop_source stopSource() { return m_ss; }
        /// @brief Get a stop token for the associated source
        std::stop_token stopToken() const { return m_ss.get_token(); }
        /// @brief Access the future of the consuming thread
        std::future<TaskStatus> &consumerStatus() { return m_consumerStatus; }
#endif
        /// @brief Access the async queue
        AsyncQueue<T> &queue() { return m_queue; }

        /// @name Push methods
        /// The push methods take a const reference (copy) or an rvalue reference (move) to a value
        /// to add to the queue. For both version a lock can also be provided. In this case it
        /// *must* be the queue's own lock. If a lock isn't provided the @ref lock method will be
        /// used to supply one.
        /// If the stop has been requested the push will be ignored and false will be returned.
        /// @{
        bool push(const T &value) { return push(value, lock()); }
        bool push(const T &value, const lock_t &lock);
        bool push(T &&value) { return push(std::move(value), lock()); }
        bool push(T &&value, const lock_t &lock);
        /// @}

        /**
         * @brief Extract the first element of the queue
         *
         * The element is removed from the queue and returned. If the queue is empty the optional
         * will not be filled. Thread safe
         */
        std::optional<T> extract() { return m_queue.extract(); }

        /**
         * @brief Extract the first element of the queue
         *
         * The element is removed from the queue and returned. If the queue is empty the optional
         * will not be filled. The lock provided *must* be for the correct mutex and locked.
         *
         * @param lock Should be an already acquired lock to the correct mutex
         */
        std::optional<T> extract(const lock_t &lock) { return m_queue.extract(lock); }

        /// @brief Get the number of elements in the queue
        std::size_t size() const { return m_queue.size(); }

        /// @brief Get the number of elements in the queue
        std::size_t size(const lock_t &lock) const { return m_queue.size(lock); }

        /// @brief Is the queue empty?
        bool empty() const { return m_queue.empty(); }

        /// @brief Is the queue empty?
        bool empty(const lock_t &lock) const { return m_queue.empty(lock); }

#ifdef AsyncQueue_MULTITHREAD
        template <typename F, typename... Args>
            requires Producer<F, T, Args...>
        std::future<TaskStatus> loopProducer(F &&f, Args &&...args) {
            return m_queue.loopProducer(m_ss, std::forward<F>(f), std::forward<Args>(args)...);
        }

        template <concepts::Duration D, typename F, typename... Args>
            requires Producer<F, T, Args...>
        std::future<TaskStatus> loopProducer(const D &d, F &&f, Args &&...args) {
            return m_queue.loopProducer(m_ss, d, std::forward<F>(f), std::forward<Args>(args)...);
        }
#endif

    private:
#ifdef AsyncQueue_MULTITHREAD
        TaskStatus consumerThread();
        std::stop_source m_ss;
#endif
        AsyncQueue<T> m_queue;
        IConsumer<T> *m_consumer;
        std::unique_ptr<IConsumer<T>> m_consumerOwning;
#ifdef AsyncQueue_MULTITHREAD
        std::future<TaskStatus> m_consumerStatus;
#endif
    };
} // namespace AsyncQueue

#ifdef AsyncQueue_MULTITHREAD
#include "AsyncQueue/ManagedQueue.ixx"
#else
#include "AsyncQueue/ManagedQueue_noMT.ixx"
#endif

#endif //> !ASYNCQUEUE_MANAGEDQUEUE_HXX