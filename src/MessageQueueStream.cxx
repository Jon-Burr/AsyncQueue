#include "AsyncQueue/MessageQueueStream.hxx"

namespace AsyncQueue {
    MessageQueueStream::MessageQueueStream() : std::ostream(&m_buffer) {}

    MessageQueueStream::MessageQueueStream(
            MessageQueue &queue, MessageLevel lvl, const std::string &source)
            : std::ostream(&m_buffer), m_buffer(queue, lvl, source) {}

    MessageQueueStream::MessageQueueStream(MessageQueueStream &&other)
            : std::ostream(&m_buffer), m_buffer(std::move(other.m_buffer)) {}

    MessageQueueStream::~MessageQueueStream() { flush(); }
} // namespace AsyncQueue