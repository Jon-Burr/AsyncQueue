#ifndef ASYNCQUEUE_TASKSTATUS_H
#define ASYNCQUEUE_TASKSTATUS_H

#include <condition_variable>
#include <mutex>

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

    class JobStatus {
    public:
        /// @brief Is the job aborting
        bool isAborted() const;
        /**
         * @brief Tell the job to go into an abort status
         *
         * All listeners to the condition variable will be notified
         */
        void abort();

        /// The calling thread waits until the job is aborted
        void waitForAbort();
        /// @brief Get the mutex
        std::mutex &mutex() const { return m_mutex; }
        /// @brief Get the condition variable
        std::condition_variable &cv() { return m_cv; }
        /// @brief Acquire a lock
        std::unique_lock<std::mutex> lock() const { return std::unique_lock<std::mutex>(m_mutex); }

    private:
        bool m_abort{false};
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
    };
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_TASKSTATUS_H