#ifndef ASYNCQUEUE_THREADMANAGER_HXX
#define ASYNCQUEUE_THREADMANAGER_HXX

#include "AsyncQueue/MessageComponent.hxx"
#include "AsyncQueue/TaskStatus.hxx"
#include "AsyncQueue/concepts.hxx"
#include "AsyncQueue/packManipulation.hxx"
#include "AsyncQueue/utils.hxx"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <type_traits>

// TODO: std::condition_variable_any loops

namespace AsyncQueue {

    /**
     * @brief Class responsible for managing the execution of threads
     *
     * The main thing that this class provides is the way to coordinate multiple threads so that
     * they can respond to an abort signal. This is done either through a condition_variable or
     * through a shared_future.
     */
    class ThreadManager : public MessageComponent {
    public:
        ThreadManager(MessageSource &&msg);

        /// @brief Check if the manager is already aborted
        bool isAborted() const;

        /// @brief Cue the manager to send the abort signal to all of its managed threads
        ///
        /// Note, this is idempotent: calling abort on an already aborted manager is a no-op
        void abort();

        /// @brief Wait for the manager to be aborted
        void wait() const;

        /// @brief Get a copy of the shared_future for the abort signal
        std::shared_future<void> getAbortFuture() const { return m_abortFuture; }

        /// @brief Create a thread that causes an abort after a set period of time
        template <concepts::Duration T> std::future<bool> setTimeout(const T &duration);

        /// @brief Create a thread that causes an abort after a set time point
        template <concepts::TimePoint T> std::future<bool> setTimeout(const T &timeout);

        template <typename F, typename... Args>
        requires concepts::Loopable<F, Args...> std::future<TaskStatus> loop(
                F &&f, Args &&... args);

        template <concepts::Duration D, typename F, typename... Args>
        requires concepts::Loopable<F, Args...> std::future<TaskStatus> loop(
                const D &heartbeat, F &&f, Args &&... args);

        template <typename F, typename... Args>
        requires concepts::Loopable<F, Args...> std::future<TaskStatus> loop(
                std::condition_variable &cv, F &&f, Args &&... args);

        template <concepts::Duration D, typename F, typename... Args>
        requires concepts::Loopable<F, Args...> std::future<TaskStatus> loop(
                std::condition_variable &cv, const D &heartbeat, F &&f, Args &&... args);

        /// @brief RAII-type class that maintains a reference to a condition_variable
        ///
        /// Used to ensure that the manager does not outlive a condition_variable that it will later
        /// attempt to notify. Only valid for condition_variable and condition_variable_any
        template <typename CV> class CVReference {
            friend class ThreadManager;

        public:
            ~CVReference();
            CVReference(CVReference &&other);
            CVReference(const CVReference &other) = delete;

        private:
            CVReference(ThreadManager *mgr, CV *cv);
            ThreadManager *m_mgr;
            CV *m_cv;
        };

        /// @brief Reference a condition variable to be notified when the manager is aborted
        CVReference<std::condition_variable> registerConditionVariable(std::condition_variable &cv);
        /// @brief Reference a condition variable to be notified when the manager is aborted
        CVReference<std::condition_variable_any> registerConditionVariable(
                std::condition_variable_any &cv);

    private:
        /// @brief Check if the manager is already aborted
        /// @param The lock on the abort mutex
        bool isAborted(const std::unique_lock<std::shared_mutex> &) const;
        bool isAborted(const std::shared_lock<std::shared_mutex> &) const;
        /// @brief The actual function that performs the timeout
        template <concepts::TimePoint T> bool doTimeout(const T &timeout);

        template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableVoid<F, Args...> TaskStatus
        doLoop(const D &heartbeat, F f, std::tuple<Args...> args);

        template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableTask<F, Args...> TaskStatus
        doLoop(const D &heartbeat, F f, std::tuple<Args...> args);

        template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableVoid<F, Args...> TaskStatus
        doLoop(std::condition_variable &cv, const D &heartbeat, F f, std::tuple<Args...> args);

        template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableTask<F, Args...> TaskStatus
        doLoop(std::condition_variable &cv, const D &heartbeat, F f, std::tuple<Args...> args);

        void increaseRefCounter(std::condition_variable *cv);
        void increaseRefCounter(std::condition_variable_any *cv);
        void decreaseRefCounter(std::condition_variable *cv);
        void decreaseRefCounter(std::condition_variable_any *cv);

        mutable std::shared_mutex m_abortMutex;
        std::promise<void> m_abortPromise;
        std::shared_future<void> m_abortFuture;
        // Reference counters for CV types
        std::mutex m_cvMutex;
        std::mutex m_cvAnyMutex;
        std::map<std::condition_variable *, std::size_t> m_cvRefCounter;
        std::map<std::condition_variable_any *, std::size_t> m_cvAnyRefCounter;
    };

} // namespace AsyncQueue

#include "AsyncQueue/ThreadManager.ixx"

#endif //> !ASYNCQUEUE_THREADMANAGER_HXX