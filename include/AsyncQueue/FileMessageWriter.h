#ifndef ASYNCQUEUE_FILEMESSAGEWRITER_H
#define ASYNCQUEUE_FILEMESSAGEWRITER_H

#include "AsyncQueue/MessageWriter.h"

#include <fstream>

namespace AsyncQueue {
    /**
     * @brief Message writer that writes to an output file
     */
    class FileMessageWriter : public MessageWriter {
    public:
        FileMessageWriter(
                const std::string &filename, MessageLevel lvl = MessageLevel::VERBOSE,
                std::ios_base::openmode mode = std::ios_base::trunc);
        FileMessageWriter(
                const std::string &filename, formatter_t format,
                MessageLevel lvl = MessageLevel::VERBOSE,
                std::ios_base::openmode mode = std::ios_base::trunc);

    private:
        std::ofstream m_ofs;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_FILEMESSAGEWRITER_H