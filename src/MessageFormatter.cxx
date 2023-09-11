#include "AsyncQueue/MessageFormatter.hxx"

namespace AsyncQueue {

    MessageFormatter::MessageFormatter()
            : MessageFormatter(std::vector<Field>{
                      MessageFormatter::defaultNameField, MessageFormatter::defaultLevelField,
                      MessageFormatter::defaultTimeField, MessageFormatter::defaultMessageField

              }) {}

    MessageFormatter::MessageFormatter(
            const std::vector<Field> &fields, const std::string &sep, bool repeatInfo)
            : m_fields(fields), m_sep(sep), m_repeatInfo(repeatInfo) {}

    std::string MessageFormatter::formatField(const Message &message, const Field &field) {
        std::string value = "";
        switch (field.type) {
        case FieldType::Name:
            value = message.source;
            break;
        case FieldType::Level:
            value = toString(message.level);
            break;
        case FieldType::Time:
            value = formatTime(message.time, field.extra);
            break;
        case FieldType::Message:
            value = message.message;
            break;
        case FieldType::Literal:
            value = field.extra;
            break;
        }
        if (field.minLength > value.size())
            value.append(field.minLength - value.size(), ' ');
        return value;
    }

    std::string MessageFormatter::format(const Message &message) const {
        std::string value = "";
        if (m_fields.size() == 0)
            return value;
        std::string prefix;
        std::string suffix;
        bool seenMsg = false;
        // TODO: This doesn't allow for multiple copies of message
        for (const Field &field : m_fields) {
            if (!seenMsg) {
                if (!prefix.empty())
                    prefix += m_sep;
                if (field.type == FieldType::Message)
                    seenMsg = true;
                else
                    prefix += formatField(message, field);
            } else
                suffix += m_sep + formatField(message, field);
        }
        std::size_t pos = 0;
        while (true) {
            std::size_t next = message.message.find('\n', pos);
            value += prefix + message.message.substr(pos, next - pos) + suffix + '\n';
            if (next >= message.message.size() - 1)
                break;
            else
                pos = next + 1;
        }
        return value;
    }

} // namespace AsyncQueue