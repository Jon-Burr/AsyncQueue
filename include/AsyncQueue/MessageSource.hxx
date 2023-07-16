#ifndef ASYNCQUEUE_MESSAGESOURCE_HXX
#define ASYNCQUEUE_MESSAGESOURCE_HXX

#include "AsyncQueue/Fwd.hxx"
#include "AsyncQueue/Message.hxx"
#include "AsyncQueue/MessageQueueStream.hxx"

#include <sstream>
#include <string>
#include <vector>

namespace AsyncQueue {
    /// @brief Thread-safe source of messages
    ///
    /// Behaves similar to an output stream. The first thing passed to it must be a level which
    /// triggers the crreation of a MessageQueueStream which handles the creation of the messages.
    /// When the builder goes out of scope all messages it contains are pushed to the queue.
    class MessageSource {
    public:
        /// @brief Create the source
        /// @param name The name of the source, can be included in messages
        /// @param queue The message queue to use
        /// @param lvl The output level. Only messages with a greater or equal severity will be
        /// output
        MessageSource(
                const std::string &name, MessageQueue &queue,
                MessageLevel lvl = MessageLevel::INFO);

        /// @brief Create a new subsource
        /// @param subName The name of the new source will be "<our name>:<subName>"
        ///
        /// The message level will be set to the same as ours
        MessageSource createSubSource(const std::string &subName) const;
        /// @brief Create a new subsource
        /// @param subName The name of the new source will be "<our name>:<subName>"
        /// @param lvl The output level. Only messages with a greater or equal severity will be
        /// output
        MessageSource createSubSource(const std::string &subName, MessageLevel lvl) const;
        /// @brief Create a subsource named after the current thread
        MessageSource createThreadSubSource() const;

        /// @brief Begin a message of the specified severity
        MessageQueueStream operator<<(MessageLevel lvl) const { return msg(lvl); }
        /// @brief The output level
        MessageLevel outputLevel() const { return m_outputLvl; }
        /// @brief Should we output a message of this severity?
        bool testLevel(MessageLevel level) const { return m_outputLvl <= level; }
        /// @brief Begin a message of the specified severity
        MessageQueueStream msg(MessageLevel lvl) const;
        /// @name Message helpers
        /// Helper functions to produce specific message levels
        /// @{
        MessageQueueStream verboseMsg() const { return msg(MessageLevel::VERBOSE); }
        template <typename... Args> void verboseMsg(Args &&...args) const {
            if (testLevel(MessageLevel::VERBOSE))
                (verboseMsg() << ... << args);
        }
        MessageQueueStream debugMsg() const { return msg(MessageLevel::DEBUG); }
        template <typename... Args> void debugMsg(Args &&...args) const {
            if (testLevel(MessageLevel::DEBUG))
                (debugMsg() << ... << args);
        }
        MessageQueueStream infoMsg() const { return msg(MessageLevel::INFO); }
        template <typename... Args> void infoMsg(Args &&...args) const {
            if (testLevel(MessageLevel::INFO))
                (infoMsg() << ... << args);
        }
        MessageQueueStream warningMsg() const { return msg(MessageLevel::WARNING); }
        template <typename... Args> void warningMsg(Args &&...args) const {
            if (testLevel(MessageLevel::WARNING))
                (warningMsg() << ... << args);
        }
        MessageQueueStream errorMsg() const { return msg(MessageLevel::ERROR); }
        template <typename... Args> void errorMsg(Args &&...args) const {
            if (testLevel(MessageLevel::ERROR))
                (errorMsg() << ... << args);
        }
        MessageQueueStream abortMsg() const { return msg(MessageLevel::ABORT); }
        template <typename... Args> void abortMsg(Args &&...args) const {
            if (testLevel(MessageLevel::ABORT))
                (abortMsg() << ... << args);
        }
        /// @}

    private:
        MessageQueue &m_queue;
        const std::string m_name;
        const MessageLevel m_outputLvl;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGESOURCE_HXX