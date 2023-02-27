#include "AsyncQueue/MessageManager.h"
#include "AsyncQueue/MessageSource.h"
#include "AsyncQueue/MessageWriter.h"

#include <chrono>

namespace AsyncQueue {
    MessageManager::MessageManager(std::unique_ptr<IMessageWriter> writer, MessageLevel lvl)
            : m_writer(std::move(writer)), m_defaultOutputLevel(lvl) {
        m_mgr.setMsg(createSource("MsgMgr"));
        m_writerStatus =
                m_queue.loopConsumer(m_mgr, [this](Message msg) { return m_writer->consume(msg); });
    }

    MessageManager::MessageManager(MessageLevel lvl)
            : MessageManager(MessageWriter(std::cout, lvl), lvl) {}

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
} // namespace AsyncQueue
