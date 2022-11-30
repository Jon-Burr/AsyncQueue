#ifndef ASYNCQUEUE_TEEWRITER_H
#define ASYNCQUEUE_TEEWRITER_H

#include "AsyncQueue/IMessageWriter.h"

#include <memory>
#include <mutex>
#include <vector>

namespace AsyncQueue {
    /// @brief Message write that just duplicates the message and sends it to multiple other writers
    class TeeWriter : public IMessageWriter {
    public:
        TeeWriter() = default;
        TeeWriter(std::vector<std::unique_ptr<IMessageWriter>> &&writers);
        void addWriter(std::unique_ptr<IMessageWriter> writer);

        TaskStatus write(const Message &message) override;

    private:
        std::mutex m_mutex;
        std::vector<std::unique_ptr<IMessageWriter>> m_writers;
    }; //> end class TeeWriter
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_TEEWRITER_H