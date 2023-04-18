#ifndef ASYNCQUEUE_MESSAGEMANAGER_HXX
#define ASYNCQUEUE_MESSAGEMANAGER_HXX

#include "AsyncQueue/AsyncQueue.hxx"
#include "AsyncQueue/Fwd.hxx"
#include "AsyncQueue/IConsumer.hxx"
#include "AsyncQueue/Message.hxx"
#include "AsyncQueue/MessageSource.hxx"
#include "AsyncQueue/ThreadManager.hxx"

#include <future>
#include <memory>
#include <type_traits>

namespace AsyncQueue {
    class MessageManager {
    public:
        MessageManager(MessageLevel outputLvl = MessageLevel::INFO);
        MessageManager(
                std::unique_ptr<IMessageWriter> writer,
                MessageLevel outputLvl = MessageLevel::INFO);
        template <
                typename T, typename = std::enable_if_t<std::is_base_of_v<IMessageWriter, T>, void>>
        MessageManager(T &&writer, MessageLevel outputLvl = MessageLevel::INFO)
                : MessageManager(std::make_unique<T>(std::move(writer)), outputLvl) {}

        ~MessageManager();

        MessageLevel defaultOutputLevel() const { return m_defaultOutputLevel; }
        void setDefaultOutputLevel(MessageLevel lvl) { m_defaultOutputLevel = lvl; }

        MessageSource createSource(const std::string &name);
        MessageSource createSource(const std::string &name, MessageLevel lvl);

    private:
        std::unique_ptr<IMessageWriter> m_writer;
        MessageLevel m_defaultOutputLevel;
        MessageQueue m_queue;
        ThreadManager m_mgr;
        std::future<TaskStatus> m_writerStatus;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEMANAGER_HXX