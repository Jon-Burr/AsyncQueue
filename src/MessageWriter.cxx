#include "AsyncQueue/MessageWriter.hxx"
#include "AsyncQueue/MessageFormatter.hxx"

#ifdef __cpp_lib_syncbuf
#include <syncstream>
#endif

namespace AsyncQueue {
    MessageWriter::MessageWriter(std::ostream &os, MessageLevel lvl)
            : MessageWriter(os, MessageFormatter(), lvl) {}

    MessageWriter::MessageWriter(std::ostream &os, formatter_t format, MessageLevel lvl)
            : m_os(os), m_lvl(lvl), m_format(format) {}

    TaskStatus MessageWriter::consume(const Message &message) {
#ifdef __cpp_lib_syncbuf
        std::osyncstream(m_os)
#else
        m_os
#endif

                << m_format(message) << std::endl;
        return TaskStatus::CONTINUE;
    }
} // namespace AsyncQueue