namespace AsyncQueue {

    template <typename T>
    ManagedQueue<T>::ManagedQueue(std::unique_ptr<IConsumer<T>> consumer)
            : m_consumer(consumer.get()), m_consumerOwning(std::move(consumer)) {}
    template <typename T>
    ManagedQueue<T>::ManagedQueue(IConsumer<T> *consumer) : m_consumer(consumer) {}
    template <typename T>
    template <std::derived_from<IConsumer<T>> Consumer>
        requires std::move_constructible<Consumer>
    ManagedQueue<T>::ManagedQueue(Consumer &&consumer)
            : ManagedQueue(std::make_unique<Consumer>(std::move(consumer))) {}
    template <typename T> ManagedQueue<T>::~ManagedQueue() {}

    template <typename T> bool ManagedQueue<T>::push(const T &value, const lock_t &lock) {
        m_queue.push(value, lock);
        m_consumer->consume(*m_queue.extract());
        return true;
    }

    template <typename T> bool ManagedQueue<T>::push(T &&value, const lock_t &lock) {
        m_queue.push(std::move(value), lock);
        m_consumer->consume(*m_queue.extract());
        return true;
    }
} // namespace AsyncQueue