#ifndef ASYNCQUEUE_TEECONSUMER_HXX
#define ASYNCQUEUE_TEECONSUMER_HXX

#include "AsyncQueue/IConsumer.hxx"

#include <concepts>
#include <memory>
#include <type_traits>
#include <vector>

namespace AsyncQueue {
    /// @brief Consumer that takes an element and forwards it to multiple consumers
    /// @tparam T The elementt type of the queue
    template <typename T> class TeeConsumer : public IConsumer<T> {
    public:
        /// @brief Create an empty consumer
        TeeConsumer() = default;

        /// @brief Create a consumer from multiple others
        template <std::move_constructible... Cs>
        requires(std::derived_from<Cs, IConsumer<T>> &&...) TeeConsumer(Cs &&... consumers);

        /// @brief The number of contained consumers
        std::size_t size() const;

        /// @brief Add a new consumer to this
        void addConsumer(std::unique_ptr<IConsumer<T>> &&consumer);

        /// @brief Add a new consumer to thiis
        template <std::move_constructible C>
        requires std::derived_from<C, IConsumer<T>> void addConsumer(C &&consumer);

        /// @brief Add a new consumer to this that is managed outside this class
        void addConsumer(IConsumer<T> *consumer);

        /// @brief Consume single element
        TaskStatus consume(const T &element) override;

    private:
        std::vector<std::unique_ptr<IConsumer<T>>> m_owned;
        std::vector<IConsumer<T> *> m_consumers;
    };
} // namespace AsyncQueue

#include "AsyncQueue/TeeConsumer.ixx"

#endif //> !ASYNCQUEUE_TEECONSUMER_HXX