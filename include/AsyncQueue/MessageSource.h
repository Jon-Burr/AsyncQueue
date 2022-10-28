#ifndef ASYNCQUEUE_MESSAGESOURCE_H
#define ASYNCQUEUE_MESSAGESOURCE_H

#include "AsyncQueue/Message.h"
#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {
    class MessageBuilder {
    public:
        /// Create a void builder
        MessageBuilder(MessageQueue &queue);
        /// Create a builder with the specified message level
        MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl);
        /**
         * @brief Send the current message to the queue
         *
         * Note that this is equivalent to msg << std::endl;
         */
        void flush();

        /// @brief Get the current output level
        MessageLevel outputLevel() const { return m_lvl; }

        /// Pass a generic type to the underlying string stream
        template <typename T> MessageBuilder &operator<<(T &&value) {
            if (m_void)
                return *this;
            m_msg << std::forward<T>(value);
            return *this;
        }

        /// Handle endl by sending the message to the queue
        using stream_mod_t = std::ostream &(*)(std::ostream &);
        MessageBuilder &operator<<(stream_mod_t mod) {
            if (m_void)
                return *this;
            if (mod == &std::endl<std::ostream::char_type, std::ostream::traits_type>)
                flush();
            else
                // OK - this was actually a generic stream modifier, not endl
                m_msg << mod;
            return *this;
        }

    private:
        bool m_void;
        const std::string m_name;
        MessageQueue &m_queue;
        // The level  message
        const MessageLevel m_lvl;
        std::ostringstream m_msg;

    }; //> end class MessageBuilder
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
        MessageBuilder operator<<(MessageLevel level);

    private:
        std::string m_name;
        MessageQueue &m_queue;
        // Don't output messages below this level
        MessageLevel m_outputLvl;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGESOURCE_H