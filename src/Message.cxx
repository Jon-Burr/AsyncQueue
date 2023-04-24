#include "AsyncQueue/Message.hxx"

#include <algorithm>
#include <stdexcept>

namespace AsyncQueue {

    std::string toString(MessageLevel lvl) {
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

    std::istream &operator>>(std::istream &is, MessageLevel &lvl) {
        std::string s;
        is >> s;
        lvl = levelFromString(s);
        return is;
    }

    std::ostream &operator<<(std::ostream &os, MessageLevel lvl) { return os << toString(lvl); }
} // namespace AsyncQueue