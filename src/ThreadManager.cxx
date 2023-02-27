#include "AsyncQueue/ThreadManager.h"
#include "AsyncQueue/MessageSource.h"

#include <cassert>
#include <chrono>
#include <stdexcept>
#include <thread>

namespace AsyncQueue {

    ThreadManager::ThreadManager(MessageSource &&msg)
            : m_msg(std::make_unique<MessageSource>(std::move(msg))) {}

    bool ThreadManager::isAborted() const {
        std::shared_lock<std::shared_mutex> lock(m_abortedMutex);
        return m_aborted;
    }

    void ThreadManager::abort() {
        using namespace std::chrono_literals;
        std::unique_lock<std::shared_mutex> abortLock(m_abortedMutex);
        m_aborted = true;
        abortLock.unlock();
        // Notify any threads that want to abort the manager that it's too late.
        m_abortCV.notify_all();
        bool cont = true;
        std::unique_lock<std::mutex> lock(m_cvMutex);
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

    std::future<bool> ThreadManager::setTimeout(
            const std::chrono::steady_clock::time_point &until) {
        return std::async(&ThreadManager::doTimeout, this, until);
    }

    bool ThreadManager::doTimeout(const std::chrono::steady_clock::time_point &until) {
        std::shared_lock<std::shared_mutex> lock(m_abortedMutex);
        while (true) {
            switch (m_abortCV.wait_until(lock, until)) {
            case std::cv_status::no_timeout:
                if (m_aborted)
                    return false;
                break;
            case std::cv_status::timeout:
                lock.unlock();
                abort();
                return true;
            }
        }
        return false;
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
