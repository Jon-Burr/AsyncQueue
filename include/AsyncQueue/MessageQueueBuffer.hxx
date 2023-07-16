#ifndef ASYNCQUEUE_MESSAGEQUEUEBUFFER_HXX
#define ASYNCQUEUE_MESSAGEQUEUEBUFFER_HXX

#include "AsyncQueue/Message.hxx"

#include <sstream>

namespace AsyncQueue {
    /// @brief Message buffer class
    ///
    /// Acts as a basic wrapper around a stringbuf that sends completed messages to the wrapped
    /// queue on a flush. If there is no associated queue this buffer does essentially nothing.
    /// However this is likely to be inefficient as all the work to format the strings is still
    /// done.
    class MessageQueueBuffer : public std::stringbuf {
    public:
        /// @brief Create an invalid buffer. All input characters will be ignored
        MessageQueueBuffer();

        /// @brief Create the buffer
        /// @param queue The queue to write to
        /// @param lvl The message level for each message
        /// @param source The name of the message source
        MessageQueueBuffer(MessageQueue &queue, MessageLevel lvl, const std::string &source);

    protected:
        int sync() override;

    private:
        MessageQueue *const m_queue{nullptr};
        const MessageLevel m_lvl{MessageLevel::ABORT};
        const std::string m_source;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEQUEUEBUFFER_HXX