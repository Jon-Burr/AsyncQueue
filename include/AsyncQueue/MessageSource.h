#ifndef ASYNCQUEUE_MESSAGESOURCE_H
#define ASYNCQUEUE_MESSAGESOURCE_H

#include "AsyncQueue/Message.h"

#include <vector>

namespace AsyncQueue {

    /**
     * @brief Creates messages and sends them to the queue
     *
     * The queue is locked for as long as the builder exists.
     */
    class MessageBuilder {
    public:
        /// Create a void builder
        MessageBuilder(MessageQueue &queue);
        /// Create a builder with the specified message level
        MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl);
        MessageBuilder(MessageBuilder &&other);
        ~MessageBuilder() { flush(); }
        /**
         * @brief Send the current message to the queue
         *
         * Note that this is equivalent to msg << std::endl;
         */
        void flush();

        /// @brief Get the current output level
        MessageLevel level() const { return m_lvl; }

        /// Pass a generic type to the underlying string stream
        template <typename T> MessageBuilder &operator<<(T &&value) {
            if (m_void)
                return *this;
            m_empty = false;
            m_msg << std::forward<T>(value);
            return *this;
        }

        /// Handle endl by sending the message to the queue
        using stream_mod_t = std::ostream &(*)(std::ostream &);
        /// @brief Output a stream modifier
        MessageBuilder &operator<<(stream_mod_t mod) {
            if (m_void)
                return *this;
            if (mod == &std::endl<std::ostream::char_type, std::ostream::traits_type>) {
                m_messages.push_back(m_msg.str());
                // Clear the message buffer
                m_msg.str("");
                m_empty = true;
            } else
                // OK - this was actually a generic stream modifier, not endl
                m_msg << mod;
            return *this;
        }

    private:
        bool m_void;
        bool m_empty{true};
        const std::string m_name;
        MessageQueue &m_queue;
        // The message level
        const MessageLevel m_lvl;
        std::ostringstream m_msg;
        std::vector<std::string> m_messages;

    }; //> end class MessageBuilder

    /// @brief Thread-safe source of messages
    ///
    /// Behaves similarly to an output stream. The first thing passed to it must be a MessageLevel
    /// at which point it creates a MessageBuilder which handles the creation of the Message. When
    /// the builder goes out of scope the created Message is pushed to the queue.
    class MessageSource {
    public:
        /**
         * @brief Construct the message source
         * @param name The name of this source
         * @param queue The queue that the source interacts with
         * @param level Don't output messages below this level of severity
         */
        MessageSource(
                const std::string &name, MessageQueue &queue,
                MessageLevel level = MessageLevel::INFO);

        /// @brief Set the level of the next message to create
        MessageBuilder operator<<(MessageLevel level) const;

        /// @brief Get the output level
        MessageLevel outputLevel() const { return m_outputLvl; }

        /// @brief Should we output this message?
        bool testLevel(MessageLevel level) const { return m_outputLvl <= level; }

    private:
        std::string m_name;
        MessageQueue &m_queue;
        // Don't output messages below this level
        MessageLevel m_outputLvl;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGESOURCE_H
