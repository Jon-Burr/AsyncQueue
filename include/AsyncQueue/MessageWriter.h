#ifndef ASYNCQUEUE_MESSAGEWRITER_H
#define ASYNCQUEUE_MESSAGEWRITER_H

#include <iostream>
#include <mutex>
#include <ostream>
#include <string>

#include "AsyncQueue/IConsumer.h"
#include "AsyncQueue/Message.h"

namespace AsyncQueue {
    /**
     * @brief Consume messages from the queue and write them to an output stream
     *
     * Note that this class is not threadsafe - it should only run in *one* thread.
     * Making it fully threadsafe would require C++20 syncstream.
     */
    class MessageWriter : public IConsumer<Message> {
    public:
        using formatter_t = std::function<std::string(const Message &)>;
        /// @brief Create the writer
        /// @param os The stream to write to
        /// @param lvl Only write messages at or above this level
        MessageWriter(std::ostream &os, MessageLevel lvl = MessageLevel::VERBOSE);
        /// @brief Create the writer
        /// @param os The stream to write to
        /// @param format Function to convert messages to strings
        /// @param lvl Only write messages at or above this level
        MessageWriter(
                std::ostream &os, formatter_t format, MessageLevel lvl = MessageLevel::VERBOSE);

        TaskStatus consume(const Message &message) override;

    private:
        std::ostream &m_os;
        MessageLevel m_lvl;
        formatter_t m_format;
    };

} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEWRITER_H