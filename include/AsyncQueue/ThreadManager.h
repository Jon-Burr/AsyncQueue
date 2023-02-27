#ifndef ASYNCQUEUE_THREADMANAGER_H
#define ASYNCQUEUE_THREADMANAGER_H

#include "AsyncQueue/TaskStatus.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <type_traits>

namespace AsyncQueue {

    class MessageSource;
    /**
     * @brief Class to manage threads
     */
    class ThreadManager {
    public:
        ThreadManager() = default;
        /// @brief Create the manage
        /// @param msg Link the manager to a message queue so it can report its status
        ThreadManager(MessageSource &&msg);
        /// @brief Signal to all running threads that they should stop
        void abort();
        /// @brief Whether the abort signal has been sent
        bool isAborted() const;

        /// @brief Create a thread to loop a functor
        /// @tparam F The type of functor
        /// @tparam ...Args The functor argument types
        /// @param f The functor
        /// @param ...args The functor arguments
        /// @return Future for the result
        template <typename F, typename... Args> std::future<void> loop(F &&f, Args &&...args);

        /// @brief Create a thread to loop a functor which depends on a condition variable
        /// @tparam F The type of functor
        /// @tparam ...Args The functor argument types
        /// @param cv The condition variable on which the functor waits
        /// @param f The functor
        /// @param ...args The functor arguments
        /// @return Future for the functor result
        ///
        /// The condition variable will be notified if the manager is aborted.
        template <typename F, typename... Args>
        std::future<void> loop(std::condition_variable &cv, F f, Args &&...args);

        /// @brief Create a thread to loop a functor with a pause between executios
        /// @tparam Rep std::chrono::duration template parameter
        /// @tparam Ratio std::chrono::duration template parameter
        /// @tparam F The type of functor
        /// @tparam ...Args The functor argument types
        /// @param heartbeat Time to wait between executions
        /// @param f The functor
        /// @param ...args The functor arguments
        /// @return Future for the functor result
        ///
        /// The heartbeat will wait *between* functor executions, from when it finished to when it
        /// starts again, not between start times.
        template <typename Rep, typename Ratio, typename F, typename... Args>
        std::future<void> loop(std::chrono::duration<Rep, Ratio> heartbeat, F &&f, Args &&...args);

        /// @brief Create a thread to loop a task functor
        /// @tparam F The type of functor
        /// @tparam ...Args The functor argument types
        /// @param f The functor
        /// @param ...args The functor arguments
        /// @return Future for the result
        template <typename F, typename... Args>
        std::future<TaskStatus> loopTask(F &&f, Args &&...args);

        /// @brief Create a thread to loop a task functor which depends on a condition variable
        /// @tparam F The type of functor
        /// @tparam ...Args The functor argument types
        /// @param cv The condition variable on which the functor waits
        /// @param f The functor
        /// @param ...args The functor arguments
        /// @return Future for the functor result
        ///
        /// The condition variable will be notified if the manager is aborted.
        template <typename F, typename... Args>
        std::future<TaskStatus> loopTask(std::condition_variable &cv, F &&f, Args &&...args);
        /// @brief Create a thread to loop a task functor with a pause between executions
        /// @tparam Rep std::chrono::duration template parameter
        /// @tparam Ratio std::chrono::duration template parameter
        /// @tparam F The type of functor
        /// @tparam ...Args The functor argument types
        /// @param heartbeat Time to wait between executions
        /// @param f The functor
        /// @param ...args The functor arguments
        /// @return Future for the functor result
        ///
        /// The heartbeat will wait *between* functor executions, from when it finished to when it
        /// starts again, not between start times.
        template <typename Rep, typename Ratio, typename F, typename... Args>
        std::future<TaskStatus> loopTask(
                std::chrono::duration<Rep, Ratio> heartbeat, F &&f, Args &&...args);

        /// @brief Loop a function
        /// @param heartbeat Time to wait between executions
        /// @param f The function to loop
        void doLoop(std::chrono::nanoseconds heartbeat, std::function<void()> f);

        /// @brief Loop a function
        /// @param heartbeat Time to wait between executions
        /// @param cv The condition variable on which the functor waits
        /// @param f The function to loop
        void doLoop(
                std::condition_variable &cv, std::chrono::nanoseconds heartbeat,
                std::function<void()> f);

        /// @brief Loop a task function
        /// @param heartbeat Time to wait between executions
        /// @param f The function to loop
        TaskStatus doLoopTask(std::chrono::nanoseconds heartbeat, std::function<TaskStatus()> f);

        /// @brief Loop a task function
        /// @param heartbeat Time to wait between executions
        /// @param cv The condition variable on which the functor waits
        /// @param f The function to loop
        TaskStatus doLoopTask(
                std::condition_variable &cv, std::chrono::nanoseconds heartbeat,
                std::function<TaskStatus()> f);

        /// Set the message source
        void setMsg(MessageSource &&msg);

    private:
        /// @brief Add a reference to the condition variable
        void reference(std::condition_variable *cv);
        /// @brief Remove a reference to the condition variable
        void dereference(std::condition_variable *cv);

        /// Object used to print error messages
        std::unique_ptr<MessageSource> m_msg;
        /// Mutex for checking if something is aborted
        mutable std::shared_mutex m_abortedMutex;
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