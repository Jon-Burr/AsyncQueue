#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {
    MessageSource::MessageSource(const std::string &name, MessageQueue &queue, MessageLevel level)
            : m_name(name), m_queue(queue), m_outputLvl(level), m_currentLvl(level) {}

    void MessageSource::flush() {
        if (m_currentLvl < m_outputLvl)
            return;
        m_queue.push(Message{
                m_name, std::chrono::high_resolution_clock::now(), m_currentLvl, m_msg.str()});
        // Clear the message buffer
        m_msg.str("");
        m_currentLvl = m_outputLvl;
    }

    MessageSource &MessageSource::operator<<(MessageLevel level) {
        m_currentLvl = level;
        return *this;
    }
} // namespace AsyncQueue