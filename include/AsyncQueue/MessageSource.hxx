#ifndef ASYNCQUEUE_MESSAGESOURCE_HXX
#define ASYNCQUEUE_MESSAGESOURCE_HXX

#include "AsyncQueue/Fwd.hxx"
#include "AsyncQueue/Message.hxx"

#include <sstream>
#include <string>
#include <vector>

namespace AsyncQueue {
    // Forward declare
    class MessageBuilder;
    template <typename T> MessageBuilder &operator<<(MessageBuilder &b, T &&t);
    /// @brief Creates messages and sends them to the queue
    ///
    /// Multiple messages can be held and will be sent at the same time on flush.
    class MessageBuilder {
    public:
        /// Create a void builder
        MessageBuilder(MessageQueue &queue);
        /// Create a builder with the specified message level
        MessageBuilder(MessageQueue &queue, const std::string &name, MessageLevel lvl);
        /// Move constructor
        MessageBuilder(MessageBuilder &&other);
        /// Destructor flushes the stored messages
        ~MessageBuilder() { flush(); }

        /// @brief Send the current message to the queue
        void flush();

        /// @brief Get the current output level
        MessageLevel level() const { return m_lvl; }

        template <typename T> friend MessageBuilder &operator<<(MessageBuilder &, T &&);
        template <typename T> friend MessageBuilder &&operator<<(MessageBuilder &&, T &&);
        friend MessageBuilder &operator<<(MessageBuilder &, std::ostream &(*f)(std::ostream &));
        friend MessageBuilder &&operator<<(MessageBuilder &&, std::ostream &(*f)(std::ostream &));

    private:
        MessageQueue &m_queue;
        const bool m_void{false};
        bool m_empty{true};
        const std::string m_name;
        const MessageLevel m_lvl{MessageLevel::ABORT};
        std::ostringstream m_msg;
        std::vector<std::string> m_messages;
        void completeMessage();
    }; //> end class MessageBuilder

    /// @brief Thread-safe source of messages
    ///
    /// Behaves similar to an output stream. The first thing passed to it must be a level which
    /// triggers the crreation of a MessageBuilder which handles the creation of the messages. When
    /// the builder goes out of scope all messages it contains are pushed to the queue.
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

        /// @brief Begin a message of the specified severity
        MessageBuilder operator<<(MessageLevel lvl) const;
        /// @brief The output level
        MessageLevel outputLevel() const { return m_outputLvl; }
        /// @brief Should we output a message of this severity?
        bool testLevel(MessageLevel level) const { return m_outputLvl <= level; }

    private:
        MessageQueue &m_queue;
        const std::string m_name;
        const MessageLevel m_outputLvl;
    };

    // TODO - interaction with std::endl

    template <typename T> MessageBuilder &operator<<(MessageBuilder &b, T &&t) {
        if constexpr (std::is_same_v<std::decay_t<T>, std::decay_t<decltype(std::endl(b.m_msg))>>)
            if (t == &std::endl<std::ostream::char_type, std::ostream::traits_type>) {
                b.completeMessage();
                return b;
            }
        b.m_msg << t;
        return b;
    }
    template <typename T> MessageBuilder &&operator<<(MessageBuilder &&b, T &&t) {
        b.m_msg << t;
        return std::move(b);
    }

    MessageBuilder &operator<<(MessageBuilder &, std::ostream &(*f)(std::ostream &));
    MessageBuilder &&operator<<(MessageBuilder &&, std::ostream &(*f)(std::ostream &));

} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGESOURCE_HXX