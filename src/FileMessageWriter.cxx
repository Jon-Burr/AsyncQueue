#include "AsyncQueue/FileMessageWriter.hxx"
#include "AsyncQueue/MessageFormatter.hxx"

namespace AsyncQueue {
    FileMessageWriter::FileMessageWriter(
            const std::string &filename, MessageLevel lvl, std::ios_base::openmode mode)
            : FileMessageWriter(filename, MessageFormatter(), lvl, mode) {}

    FileMessageWriter::FileMessageWriter(
            const std::string &filename, formatter_t formatter, MessageLevel lvl,
            std::ios_base::openmode mode)
            : MessageWriter(m_ofs, lvl), m_ofs(filename, mode) {}
} // namespace AsyncQueue