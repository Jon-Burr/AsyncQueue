#ifndef ASYNCQUEUE_MESSAGEWRITER_H
#define ASYNCQUEUE_MESSAGEWRITER_H

#include <iostream>
#include <mutex>
#include <ostream>
#include <string>

#include "AsyncQueue/IMessageWriter.h"
#include "AsyncQueue/Message.h"
#include "AsyncQueue/ThreadManager.h"

namespace AsyncQueue {
    class MessageWriter : public IMessageWriter {
    public:
        using formatter_t = std::function<std::string(const Message &)>;
        MessageWriter(std::ostream &os, MessageLevel lvl = MessageLevel::VERBOSE);
        MessageWriter(
                std::ostream &os, formatter_t format, MessageLevel lvl = MessageLevel::VERBOSE);

        TaskStatus write(const Message &message) override;

    private:
        std::ostream &m_os;
        MessageLevel m_lvl;
        formatter_t m_format;
    };

} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEWRITER_H