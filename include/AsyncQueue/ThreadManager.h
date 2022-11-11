#ifndef ASYNCQUEUE_THREADMANAGER_H
#define ASYNCQUEUE_THREADMANAGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <optional>
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

    class MessageSource;
    class ThreadManager {
    public:
        ThreadManager() = default;
        ThreadManager(MessageSource &&msg);
        /// @brief Signal to all running threads that they should stop
        void abort();
        /// @brief Whether the abort signal has been sent
        bool isAborted() const { return m_aborted; }

        template <typename F, typename... Args> std::future<void> loop(F &&f, Args &&...args);
        template <typename F, typename... Args>
        std::future<void> loop(std::condition_variable &cv, F f, Args &&...args);
        template <typename Rep, typename Ratio, typename F, typename... Args>
        std::future<void> loop(std::chrono::duration<Rep, Ratio> heartbeat, F &&f, Args &&...args);
        template <typename F, typename... Args>
        std::future<TaskStatus> loopTask(F &&f, Args &&...args);
        template <typename F, typename... Args>
        std::future<TaskStatus> loopTask(std::condition_variable &cv, F &&f, Args &&...args);
        template <typename Rep, typename Ratio, typename F, typename... Args>
        std::future<TaskStatus> loopTask(
                std::chrono::duration<Rep, Ratio> heartbeat, F &&f, Args &&...args);

        void doLoop(std::chrono::nanoseconds heartbeat, std::function<void()> f);
        void doLoop(
                std::condition_variable &cv, std::chrono::nanoseconds heartbeat,
                std::function<void()> f);

        TaskStatus doLoopTask(std::chrono::nanoseconds heartbeat, std::function<TaskStatus()> f);
        TaskStatus doLoopTask(
                std::condition_variable &cv, std::chrono::nanoseconds heartbeat,
                std::function<TaskStatus()> f);

        /**
         * @brief Reset the status so it is no longer aborted
         *
         * Note that the user is responsible for race conditions here and ensuring all managed
         * threads have aborted before calling this.
         */
        void restart() { m_aborted = false; }

        /// Set the message source
        void setMsg(MessageSource &&msg);

    private:
        /// @brief Add a reference to the condition variable
        void reference(std::condition_variable *cv);
        /// @brief Remove a reference to the condition variable
        void dereference(std::condition_variable *cv);

        /// Object used to print error messages
        std::unique_ptr<MessageSource> m_msg;
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