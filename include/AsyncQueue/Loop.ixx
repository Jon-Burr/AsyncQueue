namespace AsyncQueue {
    template <concepts::Duration D, typename F, typename... Args>
        requires LoopingTask<F, Args...>
    std::future<TaskStatus> loop(std::stop_source ss, const D &duration, F &&f, Args &&...args) {
        return std::async(
                std::launch::async,
                [](std::stop_source ss, const D &duration, F &&f, Args &&...args) {
                    while (!ss.stop_requested()) {
                        auto next = std::chrono::steady_clock::now() + duration;
                        TaskStatus status{TaskStatus::CONTINUE};
                        try {
                            status = std::invoke(f, args...);
                        } catch (...) {
                            ss.request_stop();
                            throw;
                        }

                        switch (status) {
                        case TaskStatus::CONTINUE:
                            std::this_thread::sleep_until(next);
                            break;
                        case TaskStatus::HALT:
                            return TaskStatus::HALT;
                        case TaskStatus::ABORT:
                            ss.request_stop();
                            return TaskStatus::ABORT;
                        }
                    }
                    return TaskStatus::CONTINUE;
                },
                ss, duration, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
        requires LoopingTask<F, Args...>
    std::future<TaskStatus> loop(std::stop_source ss, F &&f, Args &&...args) {
        return std::async(
                std::launch::async,
                [](std::stop_source ss, F &&f, Args &&...args) {
                    while (!ss.stop_requested()) {
                        TaskStatus status{TaskStatus::CONTINUE};
                        try {
                            status = std::invoke(f, args...);
                        } catch (...) {
                            ss.request_stop();
                            throw;
                        }

                        switch (status) {
                        case TaskStatus::CONTINUE:
                            break;
                        case TaskStatus::HALT:
                            return TaskStatus::HALT;
                        case TaskStatus::ABORT:
                            ss.request_stop();
                            return TaskStatus::ABORT;
                        }
                    }
                    return TaskStatus::CONTINUE;
                },
                ss, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <concepts::Duration D, typename F, typename... Args>
        requires std::invocable<F, Args...> && (!LoopingTask<F, Args...>)
    std::future<void> loop(std::stop_source ss, const D &duration, F &&f, Args &&...args) {
        return std::async(
                std::launch::async,
                [](std::stop_source ss, const D &duration, F &&f, Args &&...args) {
                    while (!ss.stop_requested()) {
                        auto next = std::chrono::steady_clock::now() + duration;
                        TaskStatus status{TaskStatus::CONTINUE};
                        try {
                            std::invoke(f, args...);
                        } catch (...) {
                            ss.request_stop();
                            throw;
                        }
                        std::this_thread::sleep_until(next);
                    }
                },
                ss, duration, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
        requires std::invocable<F, Args...> && (!LoopingTask<F, Args...>)
    std::future<void> loop(std::stop_source ss, F &&f, Args &&...args) {
        return std::async(
                std::launch::async,
                [](std::stop_source ss, F &&f, Args &&...args) {
                    while (!ss.stop_requested()) {
                        try {
                            std::invoke(f, args...);
                        } catch (...) {
                            ss.request_stop();
                            throw;
                        }
                    }
                },
                ss, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <concepts::Duration D>
    std::future<bool> setTimeout(std::stop_source ss, const D &duration) {
        return setTimeout(ss, std::chrono::steady_clock::now() + duration);
    }

    template <concepts::TimePoint T>
    std::future<bool> setTimeout(std::stop_source ss, const T &timepoint) {
        return std::async(
                std::launch::async,
                [](std::stop_source ss, T timepoint) {
                    std::mutex m;
                    std::condition_variable_any cv;
                    auto l = std::unique_lock(m);
                    bool result = cv.wait_until(
                            l, ss.get_token(), timepoint, [ss]() { return ss.stop_requested(); });
                    if (!ss.stop_requested())
                        ss.request_stop();
                    return !result;
                },
                ss, timepoint);
    }
} // namespace AsyncQueue