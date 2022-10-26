#include "AsyncQueue/ThreadManager.h"

#include <cassert>

namespace AsyncQueue {
    void ThreadManager::abort() {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        if (m_aborted)
            // Nothing to do, the thread is already aborting
            return;
        m_aborted = true;
        for (std::pair<std::condition_variable *const, std::size_t> &cv : m_cvCounter) {
            // Only notify if we still have a reference
            if (cv.second > 0)
                cv.first->notify_all();
        }
    }

    void ThreadManager::reference(std::condition_variable *cv) {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        ++m_cvCounter[cv];
    }

    void ThreadManager::dereference(std::condition_variable *cv) {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        std::size_t &counter = m_cvCounter[cv];
        assert(counter != 0);
        --counter;
    }
} // namespace AsyncQueue