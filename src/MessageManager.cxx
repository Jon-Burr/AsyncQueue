#include "AsyncQueue/MessageManager.hxx"
#include "AsyncQueue/MessageWriter.hxx"

#include <iostream>

namespace AsyncQueue {
    MessageManager::MessageManager(MessageLevel outputLvl)
            : MessageManager(MessageWriter(std::cout, outputLvl), outputLvl) {}

    MessageManager::MessageManager(std::unique_ptr<IMessageWriter> writer, MessageLevel outputLvl)
            : m_writer(std::move(writer)), m_defaultOutputLevel(outputLvl),
              m_mgr(createSource("MessageMgr")),
              m_writerStatus(m_queue.loopConsumer(m_mgr, std::ref(*m_writer))) {}

    MessageManager::~MessageManager() {
        m_mgr.abort();
        m_writerStatus.wait();
        // We may still have messages on the queue, still emit these but synchronously now
        auto lock_ = m_queue.lock();
        while (auto msg = m_queue.extract(lock_))
            m_writer->consume(msg.value());
    }

    MessageSource MessageManager::createSource(const std::string &name) {
        return createSource(name, m_defaultOutputLevel);
    }

    MessageSource MessageManager::createSource(const std::string &name, MessageLevel lvl) {
        return MessageSource(name, m_queue, lvl);
    }
} // namespace AsyncQueue