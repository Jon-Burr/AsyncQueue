#include <functional>
#include <stdexcept>

namespace AsyncQueue {
    template <concepts::Duration T> std::future<bool> ThreadManager::setTimeout(const T &duration) {
        return setTimeout(std::chrono::steady_clock::now() + duration);
    }

    template <concepts::TimePoint T> std::future<bool> ThreadManager::setTimeout(const T &timeout) {
        return std::async(&ThreadManager::doTimeout<T>, this, timeout);
    }

    template <typename F, typename... Args>
        requires concepts::Loopable<F, Args...>
    std::future<TaskStatus> ThreadManager::loop(F &&f, Args &&...args) {
        using namespace std::chrono_literals;
        return loop(0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <concepts::Duration D, typename F, typename... Args>
        requires concepts::Loopable<F, Args...>
    std::future<TaskStatus> ThreadManager::loop(const D &heartbeat, F &&f, Args &&...args) {
        TaskStatus (ThreadManager::*impl)(
                const D &, std::decay_t<F>, std::tuple<std::decay_t<Args>...>) =
                &ThreadManager::doLoop;
        return std::async(impl, this, heartbeat, f, std::make_tuple(args...));
    }

    template <typename F, typename... Args>
        requires concepts::Loopable<F, Args...>
    std::future<TaskStatus> ThreadManager::loop(
            std::condition_variable &cv, F &&f, Args &&...args) {
        using namespace std::chrono_literals;
        return loop(cv, 0s, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <concepts::Duration D, typename F, typename... Args>
        requires concepts::Loopable<F, Args...>
    std::future<TaskStatus> ThreadManager::loop(
            std::condition_variable &cv, const D &heartbeat, F &&f, Args &&...args) {
        TaskStatus (ThreadManager::*impl)(
                std::condition_variable &, const D &, std::decay_t<F>,
                std::tuple<std::decay_t<Args>...>) = &ThreadManager::doLoop;
        return std::async(impl, this, std::ref(cv), heartbeat, f, std::make_tuple(args...));
    }

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
        requires concepts::LoopableVoid<F, Args...>
    TaskStatus ThreadManager::doLoop(const D &heartbeat, F f, std::tuple<Args...> args) {
        MessageSource source = msgSource().createThreadSubSource();
        std::size_t loopCounter = 0;
        source.debugMsg("Begin loop");
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            source.verboseMsg("Loop #", loopCounter++);
            try {
                std::apply(f, args);
            } catch (const std::exception &e) {
                source.abortMsg("Caught exception ", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        source.debugMsg("End loop");
        return TaskStatus::CONTINUE;
    }

    template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableTask<F, Args...>
    TaskStatus ThreadManager::doLoop(const D &heartbeat, F f, std::tuple<Args...> args) {
        MessageSource source = msgSource().createThreadSubSource();
        std::size_t loopCounter = 0;
        source.debugMsg("Begin loop");
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            source.verboseMsg("Loop #", loopCounter++);
            try {
                std::apply(f, args);
            } catch (const std::exception &e) {
                source.abortMsg("Caught exception ", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            auto stat = abortFuture.wait_for(heartbeat);

            if (stat == std::future_status::ready)
                break;
        }
        source.debugMsg("End loop");
        return TaskStatus::CONTINUE;
    }

    template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableVoid<F, Args...>
    TaskStatus ThreadManager::doLoop(
            std::condition_variable &cv, const D &heartbeat, F f, std::tuple<Args...> args) {
        MessageSource source = msgSource().createThreadSubSource();
        std::size_t loopCounter = 0;
        source.debugMsg("Begin loop");
        std::shared_future<void> abortFuture = getAbortFuture();
        while (true) {
            source.verboseMsg("Loop #", loopCounter++);
            TaskStatus status = TaskStatus::CONTINUE;
            try {
                status = std::apply(f, args);
            } catch (const std::exception &e) {
                source.abortMsg("Caught exception ", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::ABORT:
                source.abortMsg("Task returned ABORT");
                abort();
                return TaskStatus::ABORT;
            case TaskStatus::HALT:
                source.debugMsg("Task returned HALT");
                return TaskStatus::HALT;
            case TaskStatus::CONTINUE:
                break;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        source.debugMsg("End loop");
        return TaskStatus::CONTINUE;
    }

    template <concepts::Duration D, typename F, typename... Args>
        requires concepts::LoopableTask<F, Args...>
    TaskStatus ThreadManager::doLoop(
            std::condition_variable &cv, const D &heartbeat, F f, std::tuple<Args...> args) {
        MessageSource source = msgSource().createThreadSubSource();
        std::size_t loopCounter = 0;
        source.debugMsg("Begin loop");
        std::shared_future<void> abortFuture = getAbortFuture();
        // Increment the CV reference counter
        auto cvref = registerConditionVariable(cv);
        while (true) {
            source.verboseMsg("Loop #", loopCounter++);
            TaskStatus status = TaskStatus::CONTINUE;
            try {
                status = std::apply(f, args);
            } catch (const std::exception &e) {
                source.abortMsg("Caught exception ", e.what());
                abort();
                return TaskStatus::ABORT;
            }
            switch (status) {
            case TaskStatus::ABORT:
                source.abortMsg("Task returned ABORT");
                abort();
                return TaskStatus::ABORT;
            case TaskStatus::HALT:
                source.debugMsg("Task returned HALT");
                return TaskStatus::HALT;
            case TaskStatus::CONTINUE:
                break;
            }
            if (abortFuture.wait_for(heartbeat) == std::future_status::ready)
                break;
        }
        source.debugMsg("End loop");
        return TaskStatus::CONTINUE;
    }

} // namespace AsyncQueue