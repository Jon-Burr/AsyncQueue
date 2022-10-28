#include "AsyncQueue/Message.h"
#include <algorithm>
#include <cctype>

namespace AsyncQueue {

    std::string to_string(MessageLevel lvl) {
        switch (lvl) {
        case MessageLevel::VERBOSE:
            return "VERBOSE";
        case MessageLevel::DEBUG:
            return "DEBUG";
        case MessageLevel::INFO:
            return "INFO";
        case MessageLevel::WARNING:
            return "WARNING";
        case MessageLevel::ERROR:
            return "ERROR";
        default:
            return "ABORT";
        }
    }

    MessageLevel levelFromString(std::string lvl) {
        std::transform(lvl.begin(), lvl.end(), lvl.begin(), [](unsigned char c) {
            return std::toupper(c);
        });
        if (lvl == "VERBOSE")
            return MessageLevel::VERBOSE;
        else if (lvl == "DEBUG")
            return MessageLevel::DEBUG;
        else if (lvl == "INFO")
            return MessageLevel::INFO;
        else if (lvl == "WARNING")
            return MessageLevel::WARNING;
        else if (lvl == "ERROR")
            return MessageLevel::ERROR;
        else if (lvl != "ABORT")
            throw std::invalid_argument(lvl);
        return MessageLevel::ABORT;
    }
} // namespace AsyncQueue