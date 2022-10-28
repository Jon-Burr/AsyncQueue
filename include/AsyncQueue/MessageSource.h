#ifndef ASYNCQUEUE_MESSAGESOURCE_H
#define ASYNCQUEUE_MESSAGESOURCE_H

#include "AsyncQueue/Message.h"
#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {
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

        /**
         * @brief Send the current message to the queue
         *
         * Note that this is equivalent to msg << std::endl;
         */
        void flush();

        /// @brief Get the current output level
        MessageLevel outputLevel() const { return m_outputLvl; }
        /**
         * @brief Set the output level
         *
         * Note that it's too late to set this after values have already been provided to this as
         * they will be discarded as early as possible.
         *
         * Ideally the start of every message should begin with setting its level.
         */
        void setOutputLevel(MessageLevel level) { m_outputLvl = level; }

        /// @brief Set the level of the next message to create
        MessageSource &operator<<(MessageLevel level);

        /// Pass a generic type to the underlying string stream
        template <typename T> MessageSource &operator<<(T &&value) {
            if (m_currentLvl < m_outputLvl)
                return *this;
            m_msg << std::forward<T>(value);
            return *this;
        }

        /// Handle endl by sending the message to the queue
        using stream_mod_t = std::ostream &(*)(std::ostream &);
        MessageSource &operator<<(stream_mod_t mod) {
            if (m_currentLvl < m_outputLvl)
                return *this;
            if (mod == &std::endl<std::ostream::char_type, std::ostream::traits_type>)
                flush();
            else
                // OK - this was actually a generic stream modifier, not endl
                m_msg << mod;
            return *this;
        }

    private:
        std::string m_name;
        MessageQueue &m_queue;
        // Don't output messages below this level
        MessageLevel m_outputLvl;
        // The level of the current message
        MessageLevel m_currentLvl;
        std::ostringstream m_msg;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGESOURCE_H