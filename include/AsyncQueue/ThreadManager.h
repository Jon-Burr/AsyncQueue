#ifndef ASYNCQUEUE_THREADMANAGER_H
#define ASYNCQUEUE_THREADMANAGER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <type_traits>

namespace AsyncQueue {
    /**
     * @brief Enum to describe the return value of a looping task in a thread
     */
    enum class TaskStatus {
        /// @brief The task should continue for another loop
        CONTINUE,
        /// @brief The task should halt and not execute any more loops
        HALT,
        /// @brief An error has occured and the whole job should halt
        ABORT
    };

    class ThreadManager {
    public:
        /// @brief Signal to all running threads that they should stop
        void abort();
        /// @brief Whether the abort signal has been sent
        bool isAborted() const { return m_aborted; }
        template <typename F, typename... Args> std::future<void> loop(F &&f, Args &&... args);

        template <typename F, typename... Args>
        std::future<void> loop(std::condition_variable &cv, F f, Args &&... args);
        template <typename... Args>
        std::future<TaskStatus> loopTask(std::function<TaskStatus(Args...)> f, Args &&... args);
        template <typename... Args>
        std::future<TaskStatus> loopTask(
                std::condition_variable &cv, std::function<TaskStatus(Args...)> f, Args &&... args);

        template <typename... Args> void doLoop(std::function<void(Args...)> f, Args &&... args);

    private:
        /// @brief Add a reference to the condition variable
        void reference(std::condition_variable *cv);
        /// @brief Remove a reference to the condition variable
        void dereference(std::condition_variable *cv);
#if 0
        template <typename F, typename... Args>
        void doLoopCV(std::condition_variable &cv, F f, Args &&... args);
        template <typename... Args>
        TaskStatus doLoopTask(std::function<TaskStatus(Args &&...)> f, Args &&... args);
        template <typename... Args>
        TaskStatus doLoopTaskCV(
                std::condition_variable &cv, std::function<TaskStatus(Args &&...)> f,
                Args &&... args);
#endif

        /// Whether the thread is aborted
        bool m_aborted{false};
        /// Mutex for the map of condition variables
        std::mutex m_cvMutex;
        /// Reference counter for condition variables
        std::map<std::condition_variable *, std::size_t> m_cvCounter;
    }; //> end class ThreadManager
} // namespace AsyncQueue

#include "AsyncQueue/ThreadManager.icc"
#endif //> !ASYNCQUEUE_THREADMANAGER_H