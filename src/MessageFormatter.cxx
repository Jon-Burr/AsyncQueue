#include "AsyncQueue/MessageFormatter.h"

namespace AsyncQueue {

    MessageFormatter::MessageFormatter()
            : MessageFormatter(std::vector<Field>{
                      MessageFormatter::defaultNameField, MessageFormatter::defaultLevelField,
                      MessageFormatter::defaultTimeField, MessageFormatter::defaultMessageField

              }) {}

    MessageFormatter::MessageFormatter(const std::vector<Field> &fields, const std::string &sep)
            : m_fields(fields), m_sep(sep) {}

    std::string MessageFormatter::formatField(const Message &message, const Field &field) {
        std::string value = "";
        switch (field.type) {
        case FieldType::Name:
            value = message.source;
            break;
        case FieldType::Level:
            value = to_string(message.level);
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
        auto itr = m_fields.begin();
        value += formatField(message, *itr);
        for (++itr; itr != m_fields.end(); ++itr)
            value += m_sep + formatField(message, *itr);
        return value;
    }

} // namespace AsyncQueue