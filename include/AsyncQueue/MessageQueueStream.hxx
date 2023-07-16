#ifndef ASYNCQUEUE_MESSAGEQUEUESTREAM_HXX
#define ASYNCQUEUE_MESSAGEQUEUESTREAM_HXX

#include "AsyncQueue/Message.hxx"
#include "AsyncQueue/MessageQueueBuffer.hxx"

#include <ostream>

namespace AsyncQueue {
    class MessageQueueStream : public std::ostream {
    public:
        MessageQueueStream();
        MessageQueueStream(MessageQueue &queue, MessageLevel lvl, const std::string &source);
        ~MessageQueueStream();

    private:
        MessageQueueBuffer m_buffer;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEQUEUESTREAM_HXX