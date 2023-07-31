namespace AsyncQueue {

    template <typename T> TaskStatus ManagedQueue<T>::consumerThread() {
        auto st = m_ss.get_token();
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
                    status = (*m_consumer)(*next);
                } catch (...) {
                    this->m_ss.request_stop();
                    throw;
                }

                switch (status) {
                case TaskStatus::CONTINUE:
                    break;
                case TaskStatus::HALT:
                    return TaskStatus::HALT;
                case TaskStatus::ABORT:
                    this->m_ss.request_stop();
                    return TaskStatus::ABORT;
                }
                lock_.lock();
            }
            this->cv().wait(lock_, st, [this]() { return true; });
        }
        // Now go through all remaining queue elements
        auto lock_ = this->lock();
        while (auto next = this->extract(lock_)) {
            switch (TaskStatus status = (*m_consumer)(*next)) {
            case TaskStatus::CONTINUE:
                continue;
            default:
                return status;
            }
        }
        return TaskStatus::CONTINUE;
    }

    template <typename T>
    ManagedQueue<T>::ManagedQueue(std::stop_source ss, IConsumer<T> *consumer)
            : m_ss(ss), m_consumer(consumer),
              m_consumerStatus(
                      std::async(std::launch::async, &ManagedQueue::consumerThread, this)) {}

    template <typename T>
    ManagedQueue<T>::ManagedQueue(std::stop_source ss, std::unique_ptr<IConsumer<T>> consumer)
            : m_ss(ss), m_consumer(consumer.get()), m_consumerOwning(std::move(consumer)),
              m_consumerStatus(
                      std::async(std::launch::async, &ManagedQueue::consumerThread, this)) {}

    template <typename T>
    ManagedQueue<T>::ManagedQueue(IConsumer<T> *consumer)
            : ManagedQueue(std::stop_source(), consumer) {}

    template <typename T>
    ManagedQueue<T>::ManagedQueue(std::unique_ptr<IConsumer<T>> consumer)
            : ManagedQueue(std::stop_source(), std::move(consumer)) {}
    template <typename T>
    template <std::derived_from<IConsumer<T>> Consumer>
        requires std::move_constructible<Consumer>
    ManagedQueue<T>::ManagedQueue(std::stop_source ss, Consumer &&consumer)
            : ManagedQueue(ss, std::make_unique<Consumer>(std::move(consumer))) {}

    template <typename T>
    template <std::derived_from<IConsumer<T>> Consumer>
        requires std::move_constructible<Consumer>
    ManagedQueue<T>::ManagedQueue(Consumer &&consumer)
            : ManagedQueue(std::stop_source(), std::make_unique<Consumer>(std::move(consumer))) {}

    template <typename T> ManagedQueue<T>::~ManagedQueue() { m_ss.request_stop(); }

    template <typename T> bool ManagedQueue<T>::push(const T &value, const lock_t &lock) {
        if (m_ss.stop_requested())
            return false;
        m_queue.push(value, lock);
        return true;
    }

    template <typename T> bool ManagedQueue<T>::push(T &&value, const lock_t &lock) {
        if (m_ss.stop_requested())
            return false;
        m_queue.push(std::move(value), lock);
        return true;
    }
} // namespace AsyncQueue