namespace AsyncQueue {
    template <typename T>
    template <std::move_constructible... Cs>
    requires(std::derived_from<Cs, IConsumer<T>> &&...)
            TeeConsumer<T>::TeeConsumer(Cs &&... consumers) {
        (addConsumer(std::move(consumers)), ...);
    }

    template <typename T> std::size_t TeeConsumer<T>::size() const { return m_consumers.size(); }

    template <typename T>
    void TeeConsumer<T>::addConsumer(std::unique_ptr<IConsumer<T>> &&consumer) {
        addConsumer(consumer.get());
        m_owned.push_back(std::move(consumer));
    }

    template <typename T>
    template <std::move_constructible C>
    requires std::derived_from<C, IConsumer<T>> void TeeConsumer<T>::addConsumer(C &&consumer) {
        addConsumer(std::make_unique<C>(std::move(consumer)));
    }

    template <typename T> void TeeConsumer<T>::addConsumer(IConsumer<T> *consumer) {
        m_consumers.push_back(consumer);
    }

    template <typename T> TaskStatus TeeConsumer<T>::consume(const T &element) {
        auto itr = m_consumers.begin();
        while (itr != m_consumers.end()) {
            switch ((**itr).consume(element)) {
            case TaskStatus::CONTINUE:
                ++itr;
                break;
            case TaskStatus::HALT: {
                // See if this is an owned consumer
                auto ownedItr = std::find_if(
                        m_owned.begin(), m_owned.end(),
                        [itr](const std::unique_ptr<IConsumer<T>> &ptr) {
                            return ptr.get() == *itr;
                        });
                if (ownedItr != m_owned.end())
                    m_owned.erase(ownedItr);
                itr = m_consumers.erase(itr);
                break;
            }
            case TaskStatus::ABORT:
                return TaskStatus::ABORT;
            }
        }
        // If we have no consumers left then halt
        return m_consumers.empty() ? TaskStatus::HALT : TaskStatus::CONTINUE;
    }
} // namespace AsyncQueue