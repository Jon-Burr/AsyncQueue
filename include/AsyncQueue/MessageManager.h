#ifndef ASYNCQUEUE_MESSAGEMANAGER_H
#define ASYNCQUEUE_MESSAGEMANAGER_H

#include "AsyncQueue/Message.h"
#include "AsyncQueue/ThreadManager.h"

#include <memory>

namespace AsyncQueue {
    /**
     * @brief Class to manage messaging for a job
     *
     * Has its own thread manager as typically you want messaging to exist for the entire job, for
     * example to report *why* any other managers aborted.
     */
    class MessageManager {
    public:
        /**
         * @brief Create the manager
         * @param outputLevel The default output level for sources
         *
         * Uses a message writer which just writes to std::cout
         */
        MessageManager(MessageLevel outputLevel = MessageLevel::INFO);

        /**
         * @brief Create the manager
         * @param writer The writer to use
         * @param outputLevel The default output level for sources
         */
        MessageManager(
                std::unique_ptr<IMessageWriter> writer,
                MessageLevel outputLevel = MessageLevel::INFO);

        /**
         * @brief Create the manager
         * @tparam T The type of writer to use
         * @param writer The writer to use
         * @param outputLevel The default output level for sources
         */
        template <
                typename T, typename = std::enable_if_t<std::is_base_of_v<IMessageWriter, T>, void>>
        MessageManager(T &&writer, MessageLevel outputLevel = MessageLevel::INFO)
                : MessageManager(std::make_unique<T>(std::move(writer)), outputLevel) {}

        ~MessageManager();

        /// @brief The default output level for new message sources
        MessageLevel defaultOutputLevel() const { return m_defaultOutputLevel; }
        /// @brief Set the default output level for new message sources
        void setDefaultOutputLevel(MessageLevel lvl) { m_defaultOutputLevel = lvl; }

        /**
         * @brief Create a message source
         *
         * The output level will be set from this object's default output level
         * @param name The name of the source
         */
        MessageSource createSource(const std::string &name);

        /**
         * @brief Create a message source
         *
         * @param name The name of the source
         * @param outputLevel The output level to set
         */
        MessageSource createSource(const std::string &name, MessageLevel outputLevel);

    private:
        std::unique_ptr<IMessageWriter> m_writer;
        MessageQueue m_queue;
        ThreadManager m_mgr;
        MessageLevel m_defaultOutputLevel;
        std::future<TaskStatus> m_writerStatus;
    }; //> end class MessageManager
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEMANAGER_H