#ifndef ASYNCQUEUE_MESSAGE_HXX
#define ASYNCQUEUE_MESSAGE_HXX

#include "Fwd.hxx"

#include <chrono>
#include <string>

namespace AsyncQueue {
    /// @brief Enum to indicate the severity of a message
    enum class MessageLevel { VERBOSE = 0, DEBUG = 1, INFO = 2, WARNING = 3, ERROR = 4, ABORT = 5 };

    /// @brief Convert a message level to its string representation
    /// @param lvl The level to convert
    std::string toString(MessageLevel lvl);

    /// @brief Convert a string to the corresponding message level
    /// @param lvl The string form of a message level, case insensitive
    MessageLevel levelFromString(const std::string &lvl);

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

    /// @name Comparison methods
    /// Comparison methods for the MessageLevel enum
    /// @{
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
    /// @}

} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGE_HXX