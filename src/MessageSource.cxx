#include "AsyncQueue/MessageSource.hxx"
#include "AsyncQueue/AsyncQueue.hxx"

#include <sstream>
#include <thread>

namespace AsyncQueue {

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

    MessageQueueStream MessageSource::msg(MessageLevel lvl) const {
        return testLevel(lvl) ? MessageQueueStream(m_queue, lvl, m_name) : MessageQueueStream();
    }

} // namespace AsyncQueue