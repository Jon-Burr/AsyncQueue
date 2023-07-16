#ifndef ASYNCQUEUE_MESSAGEFORMATTER_HXX
#define ASYNCQUEUE_MESSAGEFORMATTER_HXX

#include "AsyncQueue/Message.hxx"

#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace AsyncQueue {
    /// @brief Basic implementation of a class that converts a message to a string to write
    class MessageFormatter {
    public:
        /// @brief Different field types
        enum class FieldType { Name, Level, Time, Message, Literal };
        /// @brief Struct describing an individual field
        struct Field {
            FieldType type;           ///< The type of field
            std::size_t minLength{0}; ///< The minimum length (i.e. padding)
            std::string extra;        ///< Extra data used to format the field
        };

        /// @brief Default version of the source name field
        static const inline Field defaultNameField{FieldType::Name, 10, ""};
        /// @brief Default version of the level field
        static const inline Field defaultLevelField{FieldType::Level, 7, ""};
        /// @brief Default version of the time field
        static const inline Field defaultTimeField{FieldType::Time, 35, "%F %T+%+uus %Z"};
        /// @brief Default version of the message field
        static const inline Field defaultMessageField{FieldType::Message, 0, ""};

        /// @brief Create the default formatter
        MessageFormatter();
        /**
         * @brief Create a custom formatter
         *
         * @param fields A list of fields describing the order of how to write the message
         * @param sep A string that will be inserted between each non-empty field
         * @param repeatInfo If a message contains newlines, repeat the name, level and time strings
         *                   on the newline. If false the blankspace will still be kept.
         */
        MessageFormatter(
                const std::vector<Field> &fields, const std::string &sep = " ",
                bool repeatInfo = false);
        /// @brief Format the message
        std::string operator()(const Message &message) { return format(message); }
        /// @brief Format the message
        std::string format(const Message &message) const;
        /**
         * @brief Format an individual field
         * @param message The original message
         * @param field The description of the field to write
         */
        static std::string formatField(const Message &message, const Field &field);
        /**
         * @tparam The type of timepoint being used
         * @param timepoint The timepoint
         * @param format String describing the time format to use
         *
         * The time format string should be the same as the one used by std::put_time with one
         * addition. The string can contain %+n, %+u or %+m which will be replaced by a
         * representation of the nano, micro or milliseconds.
         */
        template <typename Clock>
        static std::string formatTime(
                const std::chrono::time_point<Clock> &timepoint, std::string format);
        /// @brief Convert to a function object
        operator std::function<std::string(const Message &message)>() {
            return std::function<std::string(const Message &msg)>(*this);
        }

    private:
        std::vector<Field> m_fields;
        std::string m_sep;
        bool m_repeatInfo;
    };

    template <typename Clock>
    std::string MessageFormatter::formatTime(
            const std::chrono::time_point<Clock> &timepoint, std::string format) {
        auto seconds = std::chrono::floor<std::chrono::seconds>(timepoint);
        if (format.find("%+") != std::string::npos) {
            std::size_t pos = format.find("%+");
            auto diff = timepoint - seconds;
            while (pos != std::string::npos) {
                if (pos + 2 >= format.size())
                    break;
                char code = format.at(pos + 2);
                std::size_t count;
                if (code == 'n')
                    count = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
                else if (code == 'u')
                    count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
                else if (code == 'm')
                    count = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
                else
                    break;
                format = format.replace(pos, 3, std::to_string(count));
                pos = format.find("%+", pos);
            }
        }
        std::ostringstream oss;
        std::time_t tmt = Clock::to_time_t(seconds);
        oss << std::put_time(std::localtime(&tmt), format.c_str());
        return oss.str();
    }
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_MESSAGEFORMATTER_HXX