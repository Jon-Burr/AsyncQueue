#include "AsyncQueue/ThreadManager.h"
#include "AsyncQueue/MessageSource.h"

#include <cassert>
#include <stdexcept>

namespace AsyncQueue {

    ThreadManager::ThreadManager(MessageSource &&msg)
            : m_msg(std::make_unique<MessageSource>(std::move(msg))) {}

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

    void ThreadManager::doLoop(std::function<void()> f) {
        while (!isAborted()) {
            try {
                f();
            } catch (const std::exception &e) {
                if (m_msg)
                    (*m_msg) << MessageLevel::ABORT << "Raised exception: " << e.what()
                             << std::endl;
                abort();
                return;
            }
        }
    }

    void ThreadManager::doLoop(std::condition_variable &cv, std::function<void()> f) {
        std::condition_variable *cvptr = &cv;
        reference(cvptr);
        doLoop(f);
        dereference(cvptr);
    }

    TaskStatus ThreadManager::doLoopTask(std::function<TaskStatus()> f) {
        while (!isAborted()) {

            TaskStatus status;
            try {
                status = f();
            } catch (const std::exception &e) {
                if (m_msg)
                    (*m_msg) << MessageLevel::ABORT << "Raised exception: " << e.what()
                             << std::endl;
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::CONTINUE:
                break;
            case TaskStatus::HALT:
                return TaskStatus::HALT;
            case TaskStatus::ABORT:
                abort();
                return TaskStatus::ABORT;
            }
        }
        return TaskStatus::CONTINUE;
    }

    TaskStatus ThreadManager::doLoopTask(
            std::condition_variable &cv, std::function<TaskStatus()> f) {
        std::condition_variable *cvptr = &cv;
        reference(cvptr);
        TaskStatus status = doLoopTask(f);
        dereference(cvptr);
        return status;
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

    void ThreadManager::setMsg(MessageSource &&msg) {
        m_msg = std::make_unique<MessageSource>(std::move(msg));
    }
} // namespace AsyncQueue