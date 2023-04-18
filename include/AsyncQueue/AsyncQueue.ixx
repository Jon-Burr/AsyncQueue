#include <cassert>

namespace AsyncQueue {
    template <typename T> std::mutex &AsyncQueue<T>::mutex() const { return m_mutex; }

    template <typename T> typename AsyncQueue<T>::lock_t AsyncQueue<T>::lock() const {
        return lock_t(m_mutex);
    }

    template <typename T> std::condition_variable &AsyncQueue<T>::cv() { return m_cv; }

    template <typename T> void AsyncQueue<T>::push(const T &value) { push(value, lock()); }

    template <typename T> void AsyncQueue<T>::push(const T &value, const lock_t &lock) {
        assert(lock.owns_lock() && lock.mutex() == &m_mutex);
        m_queue.push(value);
        m_cv.notify_one();
    }

    template <typename T> void AsyncQueue<T>::push(T &&value) { push(std::move(value), lock()); }

    template <typename T> void AsyncQueue<T>::push(T &&value, const lock_t &lock) {
        assert(lock.owns_lock() && lock.mutex() == &m_mutex);
        m_queue.push(std::move(value));
        m_cv.notify_one();
    }

    template <typename T> std::optional<T> AsyncQueue<T>::extract() { return extract(lock()); }

    template <typename T> std::optional<T> AsyncQueue<T>::extract(const lock_t &lock) {
        if (empty(lock))
            return std::nullopt;
        T value = std::move(m_queue.front());
        m_queue.pop();
        return std::move(value);
    }

    template <typename T> std::size_t AsyncQueue<T>::size() const { return size(lock()); }

    template <typename T> std::size_t AsyncQueue<T>::size(const lock_t &) const {
        return m_queue.size();
    }

    template <typename T> bool AsyncQueue<T>::empty() const { return empty(lock()); }

    template <typename T> bool AsyncQueue<T>::empty(const lock_t &) const {
        return m_queue.empty();
    }

    template <typename T>
    template <std::size_t I, typename F, typename... Args>
    std::future<TaskStatus> AsyncQueue<T>::loopConsumer(ThreadManager &mgr, F &&f, Args &&...args) {
        static_assert(
                detail::is_applicable_v<
                        std::decay_t<F>,
                        detail::insert_tuple_element_t<I, T &&, std::tuple<std::decay_t<Args>...>>>,
                "The functor doesn't have the correct signature!");
        TaskStatus (AsyncQueue::*fn)(std::decay_t<F>, std::decay_t<Args>...) =
                &AsyncQueue::doLoopConsumer<I>;
        return mgr.loopTaskCV(cv(), fn, this, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename T>
    template <std::size_t I, typename F, typename... Args, typename>
    std::future<TaskStatus> AsyncQueue<T>::loopProducer(ThreadManager &mgr, F &&f, Args &&...args) {
        using args_t = detail::insert_tuple_element_t<
                I, std::reference_wrapper<AsyncQueue<T>>, std::tuple<std::decay_t<Args>...>>;
        static_assert(
                detail::is_applicable_v<std::decay_t<F>, args_t>,
                "The functor signature does not match the arguments");
        static_assert(
                std::is_convertible_v<detail::apply_result_t<std::decay_t<F>, args_t>, TaskStatus>,
                "Functor does not return TaskStatus");
        using namespace std::chrono_literals;
        return loopProducer<I>(mgr, 0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename T>
    template <std::size_t I, typename Duration, typename F, typename... Args, typename>
    std::future<TaskStatus> AsyncQueue<T>::loopProducer(
            ThreadManager &mgr, const Duration &duration, F &&f, Args &&...args) {
        using args_t = detail::insert_tuple_element_t<
                I, std::reference_wrapper<AsyncQueue<T>>, std::tuple<std::decay_t<Args>...>>;
        static_assert(
                detail::is_applicable_v<std::decay_t<F>, args_t>,
                "The functor signature does not match the arguments");
        static_assert(
                std::is_convertible_v<detail::apply_result_t<std::decay_t<F>, args_t>, TaskStatus>,
                "Functor does not return TaskStatus");
        TaskStatus (AsyncQueue::*fn)(std::decay_t<F>, std::decay_t<Args>...) =
                &AsyncQueue::doLoopProducer<I>;
        return mgr.loopTask(duration, fn, this, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename T>
    template <std::size_t I, typename F, typename... Args>
    TaskStatus AsyncQueue<T>::doLoopProducer(F f, Args... args) {
        return std::apply(
                f, detail::insert_tuple_element<I>(*this, std::forward_as_tuple(args...)));
    }

    template <typename T>
    template <std::size_t I, typename F, typename... Args>
    TaskStatus AsyncQueue<T>::doLoopConsumer(F f, Args... args) {
        lock_t lock_ = lock();
        if (auto value = extract(lock_)) {
            // Unlock while the consumer deals with the value
            lock_.unlock();
            return std::apply(
                    f, detail::insert_tuple_element<I>(
                               std::move(value.value()), std::forward_as_tuple(args...)));
        } else
            cv().wait(lock_);
        return TaskStatus::CONTINUE;
    }

} // namespace AsyncQueue