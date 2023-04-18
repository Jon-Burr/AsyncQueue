#ifndef ASYNCQUEUE_TASKSTATUS_HXX
#define ASYNCQUEUE_TASKSTATUS_HXX

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
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_TASKSTATUS_HXX