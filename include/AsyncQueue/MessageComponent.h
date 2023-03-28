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

        template <typename... Args> void sendMsg(MessageLevel lvl, Args &&... args) const {
            if (msgSource().testLevel(lvl))
                (msg(lvl) << ... << args) << std::endl;
        }

        MessageBuilder verboseMsg() const { return msg(MessageLevel::VERBOSE); }
        template <typename... Args> void verboseMsg(Args &&... args) const {
            sendMsg(MessageLevel::VERBOSE, std::forward<Args>(args)...);
        }
        MessageBuilder debugMsg() const { return msg(MessageLevel::DEBUG); }
        template <typename... Args> void debugMsg(Args &&... args) const {
            sendMsg(MessageLevel::DEBUG, std::forward<Args>(args)...);
        }
        MessageBuilder infoMsg() const { return msg(MessageLevel::INFO); }
        template <typename... Args> void infoMsg(Args &&... args) const {
            sendMsg(MessageLevel::INFO, std::forward<Args>(args)...);
        }
        MessageBuilder warningMsg() const { return msg(MessageLevel::WARNING); }
        template <typename... Args> void warningMsg(Args &&... args) const {
            sendMsg(MessageLevel::WARNING, std::forward<Args>(args)...);
        }
        MessageBuilder errorMsg() const { return msg(MessageLevel::ERROR); }
        template <typename... Args> void errorMsg(Args &&... args) const {
            sendMsg(MessageLevel::ERROR, std::forward<Args>(args)...);
        }
        MessageBuilder abortMsg() const { return msg(MessageLevel::ABORT); }
        template <typename... Args> void abortMsg(Args &&... args) const {
            sendMsg(MessageLevel::ABORT, std::forward<Args>(args)...);
        }
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