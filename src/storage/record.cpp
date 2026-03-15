#include "storage/record.h"
#include <cstring>
#include <stdexcept>

// Serialize
std::vector<char> Record::serialize() const {
    std::vector<char> buffer;

    for (const auto& fieldValue : fieldValues) {
        // Write type byte
        uint8_t typeTage = static_cast<uint8_t>(fieldValue.type);
        buffer.push_back(static_cast<char>(typeTage));

        if (fieldValue.type == FieldType::INT) {
            // Write 4 bytes little-endian int
            int32_t val = fieldValue.intValue;
            char bytes[4];
            std::memcpy(bytes, &val, 4);
            buffer.insert(buffer.end(), bytes, bytes + 4);
        } else if (fieldValue.type == FieldType::TEXT) {
            // Write 4 bytes length + N string bytes
            uint32_t len = fieldValue.textVale.size();
            char lenBytes[4];
            std::memcpy(lenBytes, &len, 4);
            buffer.insert(buffer.end(), lenBytes, lenBytes + 4);
            buffer.insert(buffer.end(), fieldValue.textVale.begin(), fieldValue.textVale.end());
        }
    }

    return buffer;
}

// Deserialize
Record Record::deserialize(const char *data, uint32_t length) {
    Record record;
    uint32_t position = 0;

    while (position < length) {
        if (position + 1 > length) {
            throw std::length_error("Invalid record length");
        }

        uint8_t typeTag = static_cast<uint8_t>(data[position]);
        position++;

        if (typeTag == static_cast<uint8_t>(FieldType::INT)) {
            if (position + 4 > length) {
                throw std::length_error("Invalid record length");
            }

            int32_t value;
            std::memcpy(&value, data + position, 4);
            position += 4;
            record.fieldValues.push_back(FieldValue::makeInt(value));

        } else if (typeTag == static_cast<uint8_t>(FieldType::TEXT)) {
            if (position + 4 > length) {
                throw std::length_error("Invalid record length");
            }

            uint32_t textLen;
            std::memcpy(&textLen, data + position, 4);
            position += 4;

            if (position + textLen > length) {
                throw std::length_error("Invalid record length");
            }

            std::string text(data + position, textLen);
            position += textLen;
            record.fieldValues.push_back(FieldValue::makeText(text));

        } else {
            throw std::length_error("Invalid record type");
        }
    }

    return record;
}

// To string
std::string Record::toString() const {
    std::string result = "(";
    for (size_t i = 0; i < fieldValues.size(); i++) {
        if (fieldValues[i].type == FieldType::INT) {
            result += std::to_string(fieldValues[i].intValue);
        } else {
            result += "\"" + fieldValues[i].textVale + "\"";
        }

        if (i < fieldValues.size() - 1) {
            result += ", ";
        }
    }
    result += ")";
    return result;
}