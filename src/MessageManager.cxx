#include "AsyncQueue/MessageManager.h"
#include "AsyncQueue/MessageSource.h"

#include <chrono>

namespace AsyncQueue {
    MessageManager::MessageManager(MessageLevel lvl) : m_defaultOutputLevel(lvl) {}
    MessageManager::MessageManager(std::unique_ptr<IMessageWriter> writer, MessageLevel lvl)
            : MessageManager(lvl) {
        m_mgr.setMsg(createSource("MsgMgr"));
        setWriter(std::move(writer));
    }

    MessageManager::~MessageManager() {
        m_mgr.abort();
        if (m_writerStatus.valid())
            m_writerStatus.wait();
        if (m_writer) {
            // We may still have messages on the queue, still emit these, but synchronously now
            auto lock_ = m_queue.lock();
            while (auto msg = m_queue.extract(lock_))
                m_writer->consume(*msg);
        }
    }

    MessageSource MessageManager::createSource(const std::string &name) {
        return createSource(name, m_defaultOutputLevel);
    }

    MessageSource MessageManager::createSource(const std::string &name, MessageLevel lvl) {
        return MessageSource(name, m_queue, lvl);
    }

    std::unique_ptr<IMessageWriter> MessageManager::setWriter(
            std::unique_ptr<IMessageWriter> writer) {
        if (m_writer) {
            // Stop any existing messaging to swap the writers
            m_mgr.abort();
            m_writerStatus.wait();
            m_mgr.restart();
        }
        // Set our writer to point to the new one
        m_writer.swap(writer);
        // Start a thread if the new writer isn't null
        if (m_writer) {
            m_writerStatus = m_queue.loopConsumer(
                    m_mgr, [this](Message msg) { return m_writer->consume(msg); });
        }
        // return the old writer
        return writer;
    }

    bool MessageManager::hasWriter() const {
        using namespace std::chrono_literals;
        return m_writer && m_writerStatus.wait_for(0s) == std::future_status::timeout;
    }
} // namespace AsyncQueue
