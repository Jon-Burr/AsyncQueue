#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {

    MessageBuilder::MessageBuilder(MessageQueue &queue)
            : m_void(true), m_queue(queue), m_lvl(MessageLevel::ABORT) {}

    MessageBuilder::MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl)
            : m_void(false), m_name(name), m_queue(queue), m_lvl(lvl) {}

    void MessageBuilder::flush() {
        if (m_void || m_empty)
            return;
        m_queue.push(Message{m_name, std::chrono::system_clock::now(), m_lvl, m_msg.str()});
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
