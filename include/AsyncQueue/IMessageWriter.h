#ifndef ASYNCQUEUE_IMESSAGEWRITER_H
#define ASYNCQUEUE_IMESSAGEWRITER_H

#include "AsyncQueue/Message.h"

namespace AsyncQueue {
    /// @brief Base class for consumers of the message queue
    class IMessageWriter {
    public:
        virtual ~IMessageWriter() = default;
        TaskStatus operator()(const Message &message) { return write(message); }
        /// @brief Consume a message
        /// @param message The message to consume
        /// @return A status code to indicate the status of the output
        virtual TaskStatus write(const Message &message) = 0;

    protected:
        IMessageWriter() = default;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_IMESSAGEWRITER_H