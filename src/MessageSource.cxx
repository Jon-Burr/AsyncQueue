#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {

    MessageBuilder::MessageBuilder(MessageQueue &queue)
            : m_void(true), m_queue(queue), m_lvl(MessageLevel::ABORT), m_lock(queue.lock()) {}

    MessageBuilder::MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl)
            : m_void(false), m_name(name), m_queue(queue), m_lvl(lvl), m_lock(queue.lock()) {}

    void MessageBuilder::flush() {
        if (m_void || m_empty)
            return;
        m_queue.push(
                Message{m_name, std::chrono::high_resolution_clock::now(), m_lvl, m_msg.str()},
                m_lock);
        // Clear the message buffer
        m_msg.str("");
        m_empty = true;
    }

    MessageSource::MessageSource(const std::string &name, MessageQueue &queue, MessageLevel level)
            : m_name(name), m_queue(queue), m_outputLvl(level) {}

    MessageBuilder MessageSource::operator<<(MessageLevel level) const {
        if (testLevel(level))
            return MessageBuilder(m_queue, m_name, level);
        else
            return MessageBuilder(m_queue);
    }
} // namespace AsyncQueue