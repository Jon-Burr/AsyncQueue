#include <cassert>

namespace AsyncQueue {
    template <typename T> std::mutex &AsyncQueue<T>::mutex() const { return m_mutex; }

    template <typename T> typename AsyncQueue<T>::lock_t AsyncQueue<T>::lock() const {
        return lock_t(m_mutex);
    }

    template <typename T> std::condition_variable_any &AsyncQueue<T>::cv() { return m_cv; }

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
    template <typename F, typename... Args>
        requires Consumer<F, T, Args...>
    std::future<TaskStatus> AsyncQueue<T>::loopConsumer(
            std::stop_source ss, F &&f, Args &&...args) {
        return std::async(
                [this](std::stop_source ss, F &&f, Args &&...args) {
                    auto st = ss.get_token();
                    while (!st.stop_requested()) {
                        auto lock_ = this->lock();
                        while (auto next = this->extract(lock_)) {
                            // Unlock while the consumer consumes the value. It's also important to
                            // ensure that the queue is not locked if request_stop is called as any
                            // callbacks that need to access the queue will be unable to acquire the
                            // lock.
                            lock_.unlock();
                            TaskStatus status{TaskStatus::CONTINUE};
                            try {
                                status = std::invoke(f, *next, std::forward<Args>(args)...);
                            } catch (...) {
                                ss.request_stop();
                                throw;
                            }

                            switch (status) {
                            case TaskStatus::CONTINUE:
                                break;
                            case TaskStatus::HALT:
                                return TaskStatus::HALT;
                            case TaskStatus::ABORT:
                                ss.request_stop();
                                return TaskStatus::ABORT;
                            }
                            lock_.lock();
                        }
                        this->cv().wait(lock_, st, [this]() { return true; });
                    }
                    return TaskStatus::CONTINUE;
                },
                ss, std::forward<F>(f), std::forward<Args>(args)...);
    }

} // namespace AsyncQueue