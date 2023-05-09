#include "AsyncQueue/MessageSource.hxx"
#include "AsyncQueue/AsyncQueue.hxx"

#include <sstream>
#include <thread>

namespace AsyncQueue {
    MessageBuilder::MessageBuilder(MessageQueue &queue) : m_queue(queue), m_void(true) {}

    MessageBuilder::MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl)
            : m_queue(queue), m_name(name), m_lvl(lvl) {}

    MessageBuilder::MessageBuilder(MessageBuilder &&other)
            : m_queue(other.m_queue), m_void(other.m_void), m_empty(other.m_empty),
              m_name(other.m_name), m_lvl(other.m_lvl), m_msg(std::move(other.m_msg)),
              m_messages(std::move(other.m_messages)) {
        other.m_empty = true;
        other.m_messages.clear();
    }

    void MessageBuilder::flush() {
        if (m_void || m_empty && m_messages.empty())
            return;
        if (!m_empty)
            completeMessage();
        auto lock = m_queue.lock();
        for (const std::string &msg : m_messages)
            m_queue.push({m_name, std::chrono::system_clock::now(), m_lvl, msg}, lock);
    }

    void MessageBuilder::completeMessage() {
        m_messages.push_back(m_msg.str());
        m_msg.str("");
        m_empty = true;
    }

    MessageSource::MessageSource(
            const std::string &name, MessageQueue &queue, MessageLevel outputLvl)
            : m_queue(queue), m_name(name), m_outputLvl(outputLvl) {}

    MessageSource MessageSource::createSubSource(const std::string &subName) const {
        return createSubSource(subName, m_outputLvl);
    }
    MessageSource MessageSource::createSubSource(
            const std::string &subName, MessageLevel outputLvl) const {
        return MessageSource(m_name + ":" + subName, m_queue, outputLvl);
    }
    MessageSource MessageSource::createThreadSubSource() const {
        std::thread::id tid = std::this_thread::get_id();
        std::ostringstream oss;
        oss << "Thread#" << tid;
        return createSubSource(oss.str());
    }

    MessageBuilder MessageSource::msg(MessageLevel lvl) const {
        if (testLevel(lvl))
            return MessageBuilder(m_queue, m_name, lvl);
        else
            return MessageBuilder(m_queue);
    }

    MessageBuilder &operator<<(MessageBuilder &b, std::ostream &(*f)(std::ostream &)) {
        if (f == &std::endl<std::ostream::char_type, std::ostream::traits_type>) {
            b.completeMessage();
            return b;
        }
        b.m_msg << f;
        return b;
    }
    MessageBuilder &&operator<<(MessageBuilder &&b, std::ostream &(*f)(std::ostream &)) {
        if (f == &std::endl<std::ostream::char_type, std::ostream::traits_type>) {
            b.completeMessage();
            return std::move(b);
        }
        b.m_msg << f;
        return std::move(b);
    }
} // namespace AsyncQueue