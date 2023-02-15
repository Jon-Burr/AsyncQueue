#ifndef ASYNCQUEUE_ICONSUMER
#define ASYNCQUEUE_ICONSUMER

#include "AsyncQueue/TaskStatus.h"

/**
 * @file IConsumer.h
 *
 * Templated base class for objects which consume queue elements
 */

namespace AsyncQueue {
    /**
     * @brief Base class for objects which consume elements from a queue
     * @tparam T The type of element being consumed
     */
    template <typename T> class IConsumer {
    public:
        virtual ~IConsumer() = default;

        TaskStatus operator()(const T &element) { return consume(element); }

        /**
         * @brief Consume a single element of the queue
         * @param element The element being consumed
         */
        virtual TaskStatus consume(const T &element) = 0;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_ICONSUMER