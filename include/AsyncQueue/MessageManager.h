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
        ~MessageManager();

        /// @brief The default output level for new message sources
        MessageLevel defaultOuputLevel() const { return m_defaultOutputLevel; }
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

        /**
         * @brief Set a new writer
         * @param writer The new writer
         * @return The old writer (if any)
         *
         * The manager will be aborted and restarted and the writing thread will be joined.
         */
        std::unique_ptr<IMessageWriter> setWriter(std::unique_ptr<IMessageWriter> writer);

        /// Do we have an active writer
        bool hasWriter() const;

    private:
        ThreadManager m_mgr;
        MessageQueue m_queue;
        MessageLevel m_defaultOutputLevel;
        std::unique_ptr<IMessageWriter> m_writer;
        std::future<TaskStatus> m_writerStatus;
    }; //> end class MessageManager
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEMANAGER_H