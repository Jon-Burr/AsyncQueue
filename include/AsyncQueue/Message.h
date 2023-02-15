#ifndef ASYNCQUEUE_MESSAGE_H
#define ASYNCQUEUE_MESSAGE_H

#include <chrono>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>

#include "AsyncQueue/AsyncQueue.h"
#include "AsyncQueue/IConsumer.h"

namespace AsyncQueue {
    /// @brief Enum to indicate the severity of a message
    enum class MessageLevel { VERBOSE = 0, DEBUG = 1, INFO = 2, WARNING = 3, ERROR = 4, ABORT = 5 };

    inline bool operator<=(MessageLevel lhs, MessageLevel rhs) {
        return static_cast<std::underlying_type_t<MessageLevel>>(lhs) <=
               static_cast<std::underlying_type_t<MessageLevel>>(rhs);
    }

    inline bool operator<(MessageLevel lhs, MessageLevel rhs) {
        return static_cast<std::underlying_type_t<MessageLevel>>(lhs) <
               static_cast<std::underlying_type_t<MessageLevel>>(rhs);
    }

    inline bool operator>(MessageLevel lhs, MessageLevel rhs) {
        return static_cast<std::underlying_type_t<MessageLevel>>(lhs) >
               static_cast<std::underlying_type_t<MessageLevel>>(rhs);
    }

    inline bool operator>=(MessageLevel lhs, MessageLevel rhs) {
        return static_cast<std::underlying_type_t<MessageLevel>>(lhs) >=
               static_cast<std::underlying_type_t<MessageLevel>>(rhs);
    }

    std::string to_string(MessageLevel lvl);

    MessageLevel levelFromString(std::string lvl);

    /// @brief Struct containing a message with its context
    struct Message {
        /// @brief The name of the source which generated the message
        const std::string source;
        /// @brief The time at which it was generated
        const std::chrono::time_point<std::chrono::system_clock> time;
        /// @brief The level (verbosity) of the message
        const MessageLevel level;
        /// @brief The message
        const std::string message;
    };

    using MessageQueue = AsyncQueue<Message>;
    using IMessageWriter = IConsumer<Message>;

} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGE_H