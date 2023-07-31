#include "AsyncQueue/MessageManager.hxx"
#include "AsyncQueue/MessageWriter.hxx"

#include <iostream>

namespace AsyncQueue {
#ifdef AsyncQueue_MULTITHREAD
    MessageManager::MessageManager(
            std::stop_source ss, std::unique_ptr<IMessageWriter> writer, MessageLevel outputLvl)
            : m_queue(ss, std::move(writer)), m_defaultOutputLevel(outputLvl) {}
    MessageManager::MessageManager(std::unique_ptr<IMessageWriter> writer, MessageLevel outputLvl)
            : MessageManager(std::stop_source(), std::move(writer), outputLvl) {}
    MessageManager::MessageManager(std::stop_source ss, MessageLevel outputLvl)
            : MessageManager(ss, MessageWriter(std::cout, outputLvl), outputLvl) {}

#else
    MessageManager::MessageManager(std::unique_ptr<IMessageWriter> writer, MessageLevel outputLvl)
            : m_queue(std::move(writer)), m_defaultOutputLevel(outputLvl) {}
#endif
    MessageManager::MessageManager(MessageLevel outputLvl)
            : MessageManager(MessageWriter(std::cout, outputLvl), outputLvl) {}

    MessageSource MessageManager::createSource(const std::string &name) {
        return createSource(name, m_defaultOutputLevel);
    }

    MessageSource MessageManager::createSource(const std::string &name, MessageLevel lvl) {
        return MessageSource(name, m_queue.queue(), lvl);
    }
} // namespace AsyncQueue