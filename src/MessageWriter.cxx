#include "AsyncQueue/MessageWriter.h"
#include "AsyncQueue/MessageFormatter.h"

namespace AsyncQueue {

    MessageWriter::MessageWriter(std::ostream &os, MessageLevel lvl)
            : MessageWriter(os, MessageFormatter(), lvl) {}

    MessageWriter::MessageWriter(std::ostream &os, formatter_t format, MessageLevel lvl)
            : m_os(os), m_lvl(lvl), m_format(format) {}

    TaskStatus MessageWriter::write(const Message &message) {
        m_os << m_format(message) << std::endl;
        return TaskStatus::CONTINUE;
    }
} // namespace AsyncQueue