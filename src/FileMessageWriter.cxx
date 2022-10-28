#include "AsyncQueue/FileMessageWriter.h"
#include "AsyncQueue/MessageFormatter.h"
#include <stdexcept>
#include <system_error>

namespace AsyncQueue {
    FileMessageWriter::FileMessageWriter(
            const std::string &filename, formatter_t format, MessageLevel lvl,
            std::ios_base::openmode mode)
            : MessageWriter(m_ofs, format, lvl), m_ofs(filename, mode) {
        if (!m_ofs.is_open()) {
            throw std::system_error(
                    std::make_error_code(std::errc::no_such_file_or_directory), filename);
        }
    }

    FileMessageWriter::FileMessageWriter(
            const std::string &filename, MessageLevel lvl, std::ios_base::openmode mode)
            : FileMessageWriter(filename, MessageFormatter(), lvl, mode) {}
} // namespace AsyncQueue