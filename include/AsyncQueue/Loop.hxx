#ifdef AsyncQueue_MULTITHREAD
#ifndef ASYNCQUEUE_LOOP_HXX
#define ASYNCQUEUE_LOOP_HXX

#include "AsyncQueue/TaskStatus.hxx"
#include "AsyncQueue/concepts.hxx"

#include <concepts>
#include <functional>
#include <future>
#include <stop_token>

namespace AsyncQueue {

    template <typename F, typename... Args>
    concept LoopingTask = std::invocable<F, std::decay_t<Args>...> && requires(F f, Args... args) {
        { std::invoke(f, args...) } -> std::convertible_to<TaskStatus>;
    };

    /// @brief Loop a task function
    /// @tparam F The functor type
    /// @tparam ...Args Arguments to the functor
    /// @tparam D a std::chrono::duration
    /// @param ss The stop token controlling execution
    /// @param period The amount of time to wait between executions
    /// @param f The functor to execute
    /// @param args Any extra arguments to the functor. Will be passed by value, not forwarded. As
    ///             std::async decays its arguments, any that should be references need to be
    ///             wrapped in a std::ref or a std::cref.
    /// @return A future containing the final result of the functor
    ///
    /// The status of the stop_source is checked once per loop iteration. If the functor throws an
    /// exception the stop_source is stopped and the exception stored in the future. Otherwise the
    /// interaction depends on the return value of the functor:
    /// - TaskStatus::CONTINUE: the loop continues to the next execution
    /// - TaskStatus::HALT: the loop is ended for this functor
    /// - TaskStatus::ABORT: the loop ends and stop is requested on the source
    ///
    /// In all cases the result of the final loop is stored in the functor (so it only returns
    /// TaskStatus::CONTINUE if the stop source is cancelled elsewhere.
    ///
    /// This version will execute the function with at least @ref period time between the start of
    /// function calls.
    template <concepts::Duration D, typename F, typename... Args>
        requires LoopingTask<F, Args...>
    std::future<TaskStatus> loop(std::stop_source ss, const D &period, F &&f, Args &&...args);

    /// @brief Loop a task function
    /// @tparam F The functor type
    /// @tparam ...Args Arguments to the functor
    /// @param ss The stop token controlling execution
    /// @param f The functor to execute
    /// @param args Any extra arguments to the functor. Will be passed by value, not forwarded. As
    ///             std::async decays its arguments, any that should be references need to be
    ///             wrapped in a std::ref or a std::cref.
    /// @return A future containing the final result of the functor
    ///
    /// The status of the stop_source is checked once per loop iteration. If the functor throws an
    /// exception the stop_source is stopped and the exception stored in the future. Otherwise the
    /// interaction depends on the return value of the functor:
    /// - TaskStatus::CONTINUE: the loop continues to the next execution
    /// - TaskStatus::HALT: the loop is ended for this functor
    /// - TaskStatus::ABORT: the loop ends and stop is requested on the source
    ///
    /// In all cases the result of the final loop is stored in the functor (so it only returns
    /// TaskStatus::CONTINUE if the stop source is cancelled elsewhere.
    template <typename F, typename... Args>
        requires LoopingTask<F, Args...>
    std::future<TaskStatus> loop(std::stop_source ss, F &&f, Args &&...args);

    /// @brief Loop a function
    /// @tparam F The functor type
    /// @tparam ...Args Arguments to the functor
    /// @tparam D a std::chrono::duration
    /// @param ss The stop token controlling execution
    /// @param period The amount of time to wait between executions
    /// @param f The functor to execute
    /// @param args Any extra arguments to the functor. Will be passed by value, not forwarded. As
    ///             std::async decays its arguments, any that should be references need to be
    ///             wrapped in a std::ref or a std::cref.
    /// @return A future containing the final result of the functor
    ///
    /// The status of the stop_source is checked once per loop iteration. If the functor throws an
    /// exception the stop_source is stopped and the exception stored in the future.
    ///
    /// This version will execute the function with at least @ref period time between the start of
    /// function calls.
    template <concepts::Duration D, typename F, typename... Args>
        requires std::invocable<F, Args...> && (!LoopingTask<F, Args...>)
    std::future<void> loop(std::stop_source ss, const D &duration, F &&f, Args &&...args);

    /// @brief Loop a function
    /// @tparam F The functor type
    /// @tparam ...Args Arguments to the functor
    /// @param ss The stop token controlling execution
    /// @param f The functor to execute
    /// @param args Any extra arguments to the functor. Will be passed by value, not forwarded. As
    ///             std::async decays its arguments, any that should be references need to be
    ///             wrapped in a std::ref or a std::cref.
    /// @return A future containing the final result of the functor
    ///
    /// The status of the stop_source is checked once per loop iteration. If the functor throws an
    /// exception the stop_source is stopped and the exception stored in the future.
    template <typename F, typename... Args>
        requires std::invocable<F, Args...> && (!LoopingTask<F, Args...>)
    std::future<void> loop(std::stop_source ss, F &&f, Args &&...args);

    /// @brief Create a thread that waits for the specified period and then stops the source
    /// @param ss The source to stop
    /// @param duration How long to wait for
    /// @return A future containing whether the timeout was responsible for stopping the source
    template <concepts::Duration D>
    std::future<bool> setTimeout(std::stop_source ss, const D &duration);

    /// @brief Create a thread that waits until the specified time and then stops the source
    /// @param ss The source to stop
    /// @param timepoint When to stop
    /// @return A future containing whether the timeout was responsible for stopping the source
    template <concepts::TimePoint T>
    std::future<bool> setTimeout(std::stop_source ss, const T &timepoint);
} // namespace AsyncQueue

#include "AsyncQueue/Loop.ixx"

#endif //> !ASYNCQUEUE_LOOP_HXX
#endif //> AsyncQueue_MULTITHREAD