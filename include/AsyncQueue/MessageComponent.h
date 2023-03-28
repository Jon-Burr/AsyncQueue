/**
 * @brief Helper class for objects that have access to a message source
 */

#ifndef ASYNCQUEUE_MESSAGECOMPONENT_H
#define ASYNCQUEUE_MESSAGECOMPONENT_H

#include "AsyncQueue/Message.h"
#include "AsyncQueue/MessageSource.h"

namespace AsyncQueue {
    /**
     * @brief Interface class
     *
     * This is the most general option, where a component only has to define its own msg method.
     */
    class IMessageComponent {
    public:
        /// @brief Get the MessageSource
        virtual const MessageSource &msgSource() const = 0;
        /// @brief Get a builder for the specified level
        virtual MessageBuilder msg(MessageLevel lvl) const { return std::move(msgSource() << lvl); }

        MessageBuilder verboseMsg() const { return msg(MessageLevel::VERBOSE); }
        MessageBuilder debugMsg() const { return msg(MessageLevel::DEBUG); }
        MessageBuilder infoMsg() const { return msg(MessageLevel::INFO); }
        MessageBuilder warningMsg() const { return msg(MessageLevel::WARNING); }
        MessageBuilder errorMsg() const { return msg(MessageLevel::ERROR); }
        MessageBuilder abortMsg() const { return msg(MessageLevel::ABORT); }
    };

    class MessageComponent : virtual public IMessageComponent {
    public:
        /// @brief Create the component from its wrapped source object
        MessageComponent(MessageSource &&msg) : m_msg(std::move(msg)) {}

        const MessageSource &msgSource() const override final { return m_msg; }

    private:
        MessageSource m_msg;
    }; //> end class MessageComponent
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGECOMPONENT_H