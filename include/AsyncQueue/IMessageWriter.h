#ifndef ASYNCQUEUE_IMESSAGEWRITER_H
#define ASYNCQUEUE_IMESSAGEWRITER_H

#include "AsyncQueue/Message.h"

namespace AsyncQueue {
    class IMessageWriter {
    public:
        virtual ~IMessageWriter() = default;
        TaskStatus operator()(const Message &message) { return write(message); }
        virtual TaskStatus write(const Message &message) = 0;

    protected:
        IMessageWriter() = default;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_IMESSAGEWRITER_H