#include "AsyncQueue/ThreadManager.h"
#include "AsyncQueue/MessageSource.h"

#include <cassert>
#include <chrono>
#include <stdexcept>

namespace AsyncQueue {

    ThreadManager::ThreadManager(MessageSource &&msg)
            : m_msg(std::make_unique<MessageSource>(std::move(msg))) {}

    void ThreadManager::abort() {
        using namespace std::chrono_literals;
        std::unique_lock<std::mutex> lock(m_cvMutex);
        m_aborted = true;
        bool cont = true;
        while (cont) {
            cont = false;
            for (std::pair<std::condition_variable *const, std::size_t> &cv : m_cvCounter) {
                // Only notify if we still have a reference
                if (cv.second > 0) {
                    cv.first->notify_all();
                    cont = true;
                }
            }
            lock.unlock();
            std::this_thread::sleep_for(100ms);
            lock.lock();
        }
    }

    void ThreadManager::doLoop(std::chrono::nanoseconds heartbeat, std::function<void()> f) {
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
            std::this_thread::sleep_for(heartbeat);
        }
    }

    void ThreadManager::doLoop(
            std::condition_variable &cv, std::chrono::nanoseconds heartbeat,
            std::function<void()> f) {
        std::condition_variable *cvptr = &cv;
        reference(cvptr);
        doLoop(heartbeat, f);
        dereference(cvptr);
    }

    TaskStatus ThreadManager::doLoopTask(
            std::chrono::nanoseconds heartbeat, std::function<TaskStatus()> f) {
        static std::atomic<std::size_t> taskCounter = 0;
        std::size_t taskID = taskCounter++;
        std::size_t loopCount = 0;
        while (!isAborted()) {
            if (m_msg && m_msg->testLevel(MessageLevel::VERBOSE))
                (*m_msg) << MessageLevel::VERBOSE << "Task #" << taskID << ": Execute loop #"
                         << loopCount++ << std::endl;
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
            std::this_thread::sleep_for(heartbeat);
        }
        return TaskStatus::CONTINUE;
    }

    TaskStatus ThreadManager::doLoopTask(
            std::condition_variable &cv, std::chrono::nanoseconds heartbeat,
            std::function<TaskStatus()> f) {
        std::condition_variable *cvptr = &cv;
        reference(cvptr);
        TaskStatus status = doLoopTask(heartbeat, f);
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
