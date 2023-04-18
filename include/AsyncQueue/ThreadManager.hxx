#ifndef ASYNCQUEUE_THREADMANAGER_HXX
#define ASYNCQUEUE_THREADMANAGER_HXX

#include "AsyncQueue/MessageComponent.hxx"
#include "AsyncQueue/TaskStatus.hxx"
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
    namespace detail {
    }

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

        /// @brief Get a copy of the shared_future for the abort signal
        std::shared_future<void> getAbortFuture() const { return m_abortFuture; }

        /// @brief Create a thread that causes an abort after a set period of time
        template <typename Rep, typename Period>
        std::future<bool> setTimeout(const std::chrono::duration<Rep, Period> &duration);

        /// @brief Create a thread that causes an abort after a set time point
        template <typename Clock, typename Duration>
        std::future<bool> setTimeout(const std::chrono::time_point<Clock, Duration> &timeout);

        /// @brief Loop a function call
        /// @param f A functor object
        /// @param args The arguments to the functor
        ///
        /// Any return value to the functor will be discarded. If the functor raises an exception it
        /// will cause the manager to abort.
        template <
                typename F, typename... Args,
                // Ensure this doesn't participate when a duration is provided
                typename = std::enable_if_t<!detail::is_duration_v<F>, void>>
        std::future<void> loop(F &&f, Args &&...args);

        /// @brief Loop a function call
        /// @param heartbeat Time to wait between functor calls
        /// @param f A functor object
        /// @param args The arguments to the functor
        ///
        /// Any return value to the functor will be discarded. If the functor raises an exception it
        /// will cause the manager to abort.
        template <
                typename Duration, typename F, typename... Args,
                typename = std::enable_if_t<detail::is_duration_v<Duration>, void>>
        std::future<void> loop(const Duration &heartbeat, F &&f, Args &&...args);

        /// @brief Loop a function call
        /// @param cv A condition variable on which the functor depends.
        /// @param f A functor object
        /// @param args The arguments to the functor
        ///
        /// The condition variable should be one on which the functor depends. It will be notified
        /// when the manager is aborted.
        ///
        /// Any return value to the functor will be discarded. If the functor raises an exception it
        /// will cause the manager to abort.
        template <typename F, typename... Args>
        std::future<void> loopCV(std::condition_variable &cv, F &&f, Args &&...args);

        /// @brief Loop a function call
        /// @param f A functor object
        /// @param args The arguments to the functor
        ///
        /// Any return value to the functor will be discarded. If the functor raises an exception it
        /// will cause the manager to abort.
        ///
        /// The functor must return a TaskStatus enum. If this is HALT then the loop on this functor
        /// will be stopped. If it is ABORT then the entire manager will be aborted.
        template <
                typename F, typename... Args,
                // Ensure this doesn't participate when a duration is provided
                typename = std::enable_if_t<!detail::is_duration_v<F>, void>>
        std::future<TaskStatus> loopTask(F &&f, Args &&...args);

        /// @brief Loop a function call
        /// @param heartbeat Time to wait between functor calls
        /// @param f A functor object
        /// @param args The arguments to the functor
        ///
        /// Any return value to the functor will be discarded. If the functor raises an exception it
        /// will cause the manager to abort.
        ///
        /// The functor must return a TaskStatus enum. If this is HALT then the loop on this functor
        /// will be stopped. If it is ABORT then the entire manager will be aborted.
        template <
                typename Duration, typename F, typename... Args,
                typename = std::enable_if_t<detail::is_duration_v<Duration>, void>>
        std::future<TaskStatus> loopTask(const Duration &heartbeat, F &&f, Args &&...args);

        /// @brief Loop a function call
        /// @param cv A condition variable on which the functor depends.
        /// @param f A functor object
        /// @param args The arguments to the functor
        ///
        /// The condition variable should be one on which the functor depends. It will be notified
        /// when the manager is aborted.
        ///
        /// Any return value to the functor will be discarded. If the functor raises an exception it
        /// will cause the manager to abort.
        ///
        /// The functor must return a TaskStatus enum. If this is HALT then the loop on this functor
        /// will be stopped. If it is ABORT then the entire manager will be aborted.
        template <typename F, typename... Args>
        std::future<TaskStatus> loopTaskCV(std::condition_variable &cv, F &&f, Args &&...args);

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
        template <typename Clock, typename Duration>
        bool doTimeout(const std::chrono::time_point<Clock, Duration> &timeout);

        /// @brief The actual function that performs the loop
        template <typename Duration, typename F, typename... Args>
        void doLoop(const Duration &heartbeat, F f, std::tuple<Args...> args);

        /// @brief The actual function that performs a loop that depends on a condition variable
        template <typename F, typename... Args>
        void doLoopCV(std::condition_variable &cv, F f, std::tuple<Args...> args);

        /// @brief The actual function that performs the loop
        template <typename Duration, typename F, typename... Args>
        TaskStatus doLoopTask(const Duration &heartbeat, F f, std::tuple<Args...> args);

        /// @brief The actual function that performs a loop that depends on a condition variable
        template <typename F, typename... Args>
        TaskStatus doLoopTaskCV(std::condition_variable &cv, F f, std::tuple<Args...> args);

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