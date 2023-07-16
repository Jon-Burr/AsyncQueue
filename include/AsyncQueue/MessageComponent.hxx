/**
 * @brief Helper class for objects that have access to a message source
 */

#ifndef ASYNCQUEUE_MESSAGECOMPONENT_HXX
#define ASYNCQUEUE_MESSAGECOMPONENT_HXX

#include "AsyncQueue/Message.hxx"
#include "AsyncQueue/MessageSource.hxx"

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
        virtual MessageQueueStream msg(MessageLevel lvl) const { return msgSource().msg(lvl); }

        MessageQueueStream verboseMsg() const { return msgSource().verboseMsg(); }
        template <typename... Args> void verboseMsg(Args &&...args) const {
            msgSource().verboseMsg(std::forward<Args>(args)...);
        }
        MessageQueueStream debugMsg() const { return msgSource().debugMsg(); }
        template <typename... Args> void debugMsg(Args &&...args) const {
            msgSource().debugMsg(std::forward<Args>(args)...);
        }
        MessageQueueStream infoMsg() const { return msgSource().infoMsg(); }
        template <typename... Args> void infoMsg(Args &&...args) const {
            msgSource().infoMsg(std::forward<Args>(args)...);
        }
        MessageQueueStream warningMsg() const { return msgSource().warningMsg(); }
        template <typename... Args> void warningMsg(Args &&...args) const {
            msgSource().warningMsg(std::forward<Args>(args)...);
        }
        MessageQueueStream errorMsg() const { return msgSource().errorMsg(); }
        template <typename... Args> void errorMsg(Args &&...args) const {
            msgSource().errorMsg(std::forward<Args>(args)...);
        }
        MessageQueueStream abortMsg() const { return msgSource().abortMsg(); }
        template <typename... Args> void abortMsg(Args &&...args) const {
            msgSource().abortMsg(std::forward<Args>(args)...);
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

#endif //> !ASYNCQUEUE_MESSAGECOMPONENT_HXX