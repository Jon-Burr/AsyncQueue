#ifndef ASYNCQUEUE_FILEMESSAGEWRITER_HXX
#define ASYNCQUEUE_FILEMESSAGEWRITER_HXX

#include "AsyncQueue/MessageWriter.hxx"

#include <fstream>

namespace AsyncQueue {
    /// @brief Message writer that writes to an output file
    class FileMessageWriter : public MessageWriter {
    public:
        /// @brief Create the writer
        /// @param filename The file to open
        /// @param lvl The minimum received message level to write
        /// @param mode The mode with which to open the file
        FileMessageWriter(
                const std::string &filename, MessageLevel lvl = MessageLevel::VERBOSE,
                std::ios_base::openmode mode = std::ios_base::trunc);

        /// @brief Create the writer
        /// @param filename The file to open
        /// @param formatter The formatter to use
        /// @param lvl The minimum received message level to write
        /// @param mode The mode with which to open the file
        FileMessageWriter(
                const std::string &filename, formatter_t formatter,
                MessageLevel lvl = MessageLevel::VERBOSE,
                std::ios_base::openmode mode = std::ios_base::trunc);

    private:
        std::ofstream m_ofs;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_FILEMESSAGEWRITER_HXX