#include "AsyncQueue/MessageWriter.hxx"
#include "AsyncQueue/MessageFormatter.hxx"

#include <syncstream>

namespace AsyncQueue {
    MessageWriter::MessageWriter(std::ostream &os, MessageLevel lvl)
            : MessageWriter(os, MessageFormatter(), lvl) {}

    MessageWriter::MessageWriter(std::ostream &os, formatter_t format, MessageLevel lvl)
            : m_os(os), m_lvl(lvl), m_format(format) {}

    TaskStatus MessageWriter::consume(const Message &message) {
        std::osyncstream(m_os) << m_format(message) << std::endl;
        return TaskStatus::CONTINUE;
    }
} // namespace AsyncQueue