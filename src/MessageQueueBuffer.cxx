#include "AsyncQueue/MessageQueueBuffer.hxx"
#include "AsyncQueue/AsyncQueue.hxx"

namespace AsyncQueue {
    MessageQueueBuffer::MessageQueueBuffer() : std::stringbuf(std::ios::out) {}

    MessageQueueBuffer::MessageQueueBuffer(
            MessageQueue &queue, MessageLevel lvl, const std::string &source)
            : std::stringbuf(std::ios::out), m_queue(&queue), m_lvl(lvl), m_source(source) {}

    int MessageQueueBuffer::sync() {
        if (m_queue)
            m_queue->push(
                    {.source = m_source,
                     .time = std::chrono::system_clock::now(),
                     .level = m_lvl,
                     .message = str()});
        // Empty the buffer
        str("");
        return 0;
    }
} // namespace AsyncQueue