#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {

    MessageBuilder::MessageBuilder(MessageQueue &queue)
            : m_void(true), m_queue(queue), m_lvl(MessageLevel::ABORT) {}

    MessageBuilder::MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl)
            : m_void(false), m_name(name), m_queue(queue), m_lvl(lvl) {}

    MessageBuilder::MessageBuilder(MessageBuilder &&other)
            : m_void(other.m_void), m_empty(other.m_empty), m_name(std::move(other.m_name)),
              m_queue(other.m_queue), m_lvl(other.m_lvl), m_msg(std::move(other.m_msg)),
              m_messages(std::move(other.m_messages)) {
        other.m_empty = true;
        other.m_messages.clear();
    }

    void MessageBuilder::flush() {
        if (m_void || (m_empty && m_messages.empty()))
            return;
        if (!m_empty) {
            m_messages.push_back(m_msg.str());
            // Clear the message buffer
            m_msg.str("");
            m_empty = true;
        }
        auto lock = m_queue.lock();
        for (const std::string &msg : m_messages)
            m_queue.push(Message{m_name, std::chrono::system_clock::now(), m_lvl, msg}, lock);
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
