#ifndef ASYNCQUEUE_TEECONSUMER_H
#define ASYNCQUEUE_TEECONSUMER_H

#include "AsyncQueue/IConsumer.h"

#include <memory>
#include <type_traits>
#include <vector>

namespace AsyncQueue {
    /// @brief Consumer that takes an element and forwards it to multiple other consumers
    /// @tparam T The element type of the queue
    template <typename T> class TeeConsumer : IConsumer<T> {
    public:
        /// @brief Create a tee consumer with an empty list of children
        TeeConsumer() = default;
        /// @brief Create a tee consumer with a list of already created consumers
        /// @param consumers The consumers
        TeeConsumer(std::vector<std::unique_ptr<IConsumer<T>>> &&consumers);

        /// @brief Add a consumer to this
        /// @param consumer
        void addConsumer(std::unique_ptr<IConsumer<T>> &&consumer);
        /// @brief Add a consumer to this
        /// @tparam C The consumer type
        /// @param consumer
        template <typename C>
        std::enable_if_t<
                std::is_base_of_v<IConsumer<T>, C> && std::is_move_constructible_v<C>, void>
        addConsumer(C &&consumer);

        TaskStatus consume(const T &element) override;

    private:
        std::vector<std::unique_ptr<IConsumer<T>>> m_consumers;

    }; //> end class TeeConsumer<T>
} // namespace AsyncQueue

#include "AsyncQueue/IConsumer.icc"

#endif //> !ASYNCQUEUE_TEECONSUMER_H