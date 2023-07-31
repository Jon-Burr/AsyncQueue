#ifndef ASYNCQUEUE_MESSAGEMANAGER_HXX
#define ASYNCQUEUE_MESSAGEMANAGER_HXX

#include "AsyncQueue/Fwd.hxx"
#include "AsyncQueue/IConsumer.hxx"
#include "AsyncQueue/ManagedQueue.hxx"
#include "AsyncQueue/Message.hxx"
#include "AsyncQueue/MessageSource.hxx"

#include <concepts>
#include <future>
#include <memory>
#include <type_traits>

namespace AsyncQueue {
    class MessageManager {
    public:
#ifdef AsyncQueue_MULTITHREAD
        MessageManager(
                std::stop_source ss, std::unique_ptr<IMessageWriter> writer,
                MessageLevel outputLvl = MessageLevel::INFO);
        MessageManager(std::stop_source ss, MessageLevel outputLvl = MessageLevel::INFO);
        template <std::derived_from<IMessageWriter> T>
            requires std::move_constructible<T>
        MessageManager(
                std::stop_source ss, T &&writer, MessageLevel outputLvl = MessageLevel::INFO);
#endif
        MessageManager(
                std::unique_ptr<IMessageWriter> writer,
                MessageLevel outputLvl = MessageLevel::INFO);
        MessageManager(MessageLevel outputLvl = MessageLevel::INFO);
        template <std::derived_from<IMessageWriter> T>
            requires std::move_constructible<T>
        MessageManager(T &&writer, MessageLevel outputLvl = MessageLevel::INFO);

        MessageLevel defaultOutputLevel() const { return m_defaultOutputLevel; }
        void setDefaultOutputLevel(MessageLevel lvl) { m_defaultOutputLevel = lvl; }

        MessageSource createSource(const std::string &name);
        MessageSource createSource(const std::string &name, MessageLevel lvl);

    private:
        ManagedQueue<Message> m_queue;
        MessageLevel m_defaultOutputLevel;
    };
} // namespace AsyncQueue

#include "AsyncQueue/MessageManager.ixx"
#endif //> !ASYNCQUEUE_MESSAGEMANAGER_HXX