#include "AsyncQueue/TeeWriter.h"

namespace AsyncQueue {
    TeeWriter::TeeWriter(std::vector<std::unique_ptr<IMessageWriter>> &&writers)
            : m_writers(std::move(writers)) {}

    void TeeWriter::addWriter(std::unique_ptr<IMessageWriter> writer) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_writers.push_back(std::move(writer));
    }

    TaskStatus TeeWriter::write(const Message &message) {
        // Figure out if any of the writers asks to halt
        auto itr = m_writers.begin();
        while (itr != m_writers.end()) {
            switch ((**itr).write(message)) {
            case TaskStatus::CONTINUE:
                ++itr;
                break;
            case TaskStatus::HALT:
                itr = m_writers.erase(itr);
                break;
            case TaskStatus::ABORT:
                return TaskStatus::ABORT;
            }
        }
        // If we have no writers left then halt
        return m_writers.size() ? TaskStatus::CONTINUE : TaskStatus::HALT;
    }

} // namespace AsyncQueue