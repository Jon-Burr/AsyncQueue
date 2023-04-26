#include <functional>
#include <stdexcept>

#include <iostream>

namespace AsyncQueue {
    template <concepts::Duration T> std::future<bool> ThreadManager::setTimeout(const T &duration) {
        return setTimeout(std::chrono::steady_clock::now() + duration);
    }

    template <concepts::TimePoint T> std::future<bool> ThreadManager::setTimeout(const T &timeout) {
        return std::async(&ThreadManager::doTimeout<T>, this, timeout);
    }

    template <typename F, typename... Args>
    requires concepts::Loopable<F, Args...> std::future<TaskStatus> ThreadManager::loop(
            F &&f, Args &&... args) {
        using namespace std::chrono_literals;
        return loop(0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <concepts::Duration D, typename F, typename... Args>
    requires concepts::Loopable<F, Args...> std::future<TaskStatus> ThreadManager::loop(
            const D &heartbeat, F &&f, Args &&... args) {
        TaskStatus (ThreadManager::*impl)(
                const D &, std::decay_t<F>, std::tuple<std::decay_t<Args>...>) =
                &ThreadManager::doLoop;
        return std::async(impl, this, heartbeat, f, std::make_tuple(args...));
    }

    template <typename F, typename... Args>
    requires concepts::Loopable<F, Args...> std::future<TaskStatus> ThreadManager::loop(
            std::condition_variable &cv, F &&f, Args &&... args) {
        using namespace std::chrono_literals;
        return loop(cv, 0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <concepts::Duration D, typename F, typename... Args>
    requires concepts::Loopable<F, Args...> std::future<TaskStatus> ThreadManager::loop(
            std::condition_variable &cv, const D &heartbeat, F &&f, Args &&... args) {
        TaskStatus (ThreadManager::*impl)(
                std::condition_variable &, const D &, std::decay_t<F>,
                std::tuple<std::decay_t<Args>...>) = &ThreadManager::doLoop;
        return std::async(impl, this, std::ref(cv), f, std::make_tuple(args...));
    }

#if 0
    template <typename F, typename... Args, typename>
    std::future<void> ThreadManager::loop(F &&f, Args &&... args) {
        static_assert(
                std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
                "The functor must be invocable from the decayed arguments!");
        using namespace std::chrono_literals;
        return loop(0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename Duration, typename F, typename... Args, typename>
    std::future<void> ThreadManager::loop(const Duration &heartbeat, F &&f, Args &&... args) {
        static_assert(
                std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
                "The functor must be invocable from the decayed arguments!");
        void (ThreadManager::*fn)(
                const Duration &, std::decay_t<F>, std::tuple<std::decay_t<Args>...>) =
                &ThreadManager::doLoop;
        return std::async(fn, this, heartbeat, f, std::make_tuple(args...));
    }

    template <typename F, typename... Args>
    std::future<void> ThreadManager::loopCV(std::condition_variable &cv, F &&f, Args &&... args) {
        static_assert(
                std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
                "The functor must be invocable from the decayed arguments!");
        void (ThreadManager::*fn)(
                std::condition_variable & cv, std::decay_t<F>, std::tuple<std::decay_t<Args>...>) =
                &ThreadManager::doLoopCV;
        return std::async(fn, this, std::ref(cv), f, std::make_tuple(args...));
    }

    template <typename F, typename... Args, typename>
    std::future<TaskStatus> ThreadManager::loopTask(F &&f, Args &&... args) {
        static_assert(
                std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
                "The functor must be invocable from the decayed arguments!");
        static_assert(
                std::is_convertible_v<
                        std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>, TaskStatus>,
                "The return value of the functor must be convertible to TaskStatus!");
        using namespace std::chrono_literals;
        return loopTask(0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename Duration, typename F, typename... Args, typename>
    std::future<TaskStatus> ThreadManager::loopTask(
            const Duration &heartbeat, F &&f, Args &&... args) {
        static_assert(
                std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
                "The functor must be invocable from the decayed arguments!");
        static_assert(
                std::is_convertible_v<
                        std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>, TaskStatus>,
                "The return value of the functor must be convertible to TaskStatus!");
        TaskStatus (ThreadManager::*fn)(
                const Duration &, std::decay_t<F>, std::tuple<std::decay_t<Args>...>) =
                &ThreadManager::doLoopTask;
        return std::async(fn, this, heartbeat, f, std::make_tuple(args...));
    }

    template <typename F, typename... Args>
    std::future<TaskStatus> ThreadManager::loopTaskCV(
            std::condition_variable &cv, F &&f, Args &&... args) {
        static_assert(
                std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>,
                "The functor must be invocable from the decayed arguments!");
        static_assert(
                std::is_convertible_v<
                        std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>, TaskStatus>,
                "The return value of the functor must be convertible to TaskStatus!");
        TaskStatus (ThreadManager::*fn)(
                std::condition_variable & cv, std::decay_t<F>, std::tuple<std::decay_t<Args>...>) =
                &ThreadManager::doLoopTaskCV;
        return std::async(fn, this, std::ref(cv), f, std::make_tuple(args...));
    }
#endif
    template <concepts::TimePoint T> bool ThreadManager::doTimeout(const T &timeout) {
        if (getAbortFuture().wait_until(timeout) == std::future_status::timeout) {
            abortMsg("Abort after timeout");
            // We have to abort the manager
            abort();
            return true;
        } else
            // The manager was aborted before we could timeout
            return false;
    }

    template <concepts::Duration D, typename F, typename... Args>
    requires concepts::LoopableVoid<F, Args...> TaskStatus
    ThreadManager::doLoop(const D &heartbeat, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            try {
                std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        return TaskStatus::CONTINUE;
    }

    template <concepts::Duration D, typename F, typename... Args>
    requires concepts::LoopableTask<F, Args...> TaskStatus
    ThreadManager::doLoop(const D &heartbeat, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        while (!isAborted()) {
            try {
                std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        return TaskStatus::CONTINUE;
    }

    template <concepts::Duration D, typename F, typename... Args>
    requires concepts::LoopableVoid<F, Args...> TaskStatus ThreadManager::doLoop(
            std::condition_variable &cv, const D &heartbeat, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            TaskStatus status = TaskStatus::CONTINUE;
            try {
                status = std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::ABORT:
                abortMsg("Task returned ABORT");
                abort();
                return TaskStatus::ABORT;
            case TaskStatus::HALT:
                return TaskStatus::HALT;
            case TaskStatus::CONTINUE:
                break;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        return TaskStatus::CONTINUE;
    }

    template <concepts::Duration D, typename F, typename... Args>
    requires concepts::LoopableTask<F, Args...> TaskStatus ThreadManager::doLoop(
            std::condition_variable &cv, const D &heartbeat, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        // Increment the CV reference counter
        auto cvref = registerConditionVariable(cv);
        while (abortFuture.wait_for(heartbeat) != std::future_status::ready) {
            TaskStatus status = TaskStatus::CONTINUE;
            try {
                status = std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::ABORT:
                abortMsg("Task returned ABORT");
                abort();
                return TaskStatus::ABORT;
            case TaskStatus::HALT:
                return TaskStatus::HALT;
            case TaskStatus::CONTINUE:
                break;
            }
        }
        return TaskStatus::CONTINUE;
    }

#if 0
    template <typename Duration, typename F, typename... Args>
    void ThreadManager::doLoop(const Duration &heartbeat, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            try {
                std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                break;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
    }

    template <typename F, typename... Args>
    void ThreadManager::doLoopCV(std::condition_variable &cv, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        // Increment the CV reference counter
        auto cvref = registerConditionVariable(cv);
        while (!isAborted()) {
            try {
                std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                break;
            }
            if (isFutureReady(abortFuture))
                break;
        }
    }

    template <typename Duration, typename F, typename... Args>
    TaskStatus ThreadManager::doLoopTask(const Duration &heartbeat, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            TaskStatus status = TaskStatus::CONTINUE;
            try {
                status = std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::ABORT:
                abortMsg("Task returned ABORT");
                abort();
                return TaskStatus::ABORT;
            case TaskStatus::HALT:
                return TaskStatus::HALT;
            case TaskStatus::CONTINUE:
                break;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        return TaskStatus::CONTINUE;
    }

    template <typename F, typename... Args>
    TaskStatus ThreadManager::doLoopTaskCV(
            std::condition_variable &cv, F f, std::tuple<Args...> args) {
        std::shared_future<void> abortFuture = getAbortFuture();
        // Increment the CV reference counter
        auto cvref = registerConditionVariable(cv);
        while (!isFutureReady(abortFuture)) {
            TaskStatus status = TaskStatus::CONTINUE;
            try {
                status = std::apply(f, args);
            } catch (const std::exception &e) {
                abortMsg("Caught exception from looping thread", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::ABORT:
                abortMsg("Task returned ABORT");
                abort();
                return TaskStatus::ABORT;
            case TaskStatus::HALT:
                return TaskStatus::HALT;
            case TaskStatus::CONTINUE:
                break;
            }
        }
        return TaskStatus::CONTINUE;
    }
#endif
} // namespace AsyncQueue