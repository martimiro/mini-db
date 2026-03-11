// Record -> one row of a table like (1, "John", 25)

#ifndef RECORD_H
#define RECORD_H

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

// Field type
// Matches tokenType INT or TEXT from the parser
enum class FieldType : uint8_t {
    INT = 0,
    TEXT = 1,
};

// Field Value
// A single value in record
struct FieldValue {
    FieldType type;
    int64_t intValue;
    std::string textVale;

    static FieldValue makeInt(int32_t value) {
        FieldValue fieldValue;
        fieldValue.type = FieldType::INT;
        fieldValue.intValue = value;
        return fieldValue;
    }

    static FieldValue makeText(const std::string& value) {
        FieldValue fieldValue;
        fieldValue.type = FieldType::TEXT;
        fieldValue.textVale = value;
        return fieldValue;
    }
};

// Record
// Row of data -> a list of Field Values
// Can serialize itself to bytes and deserialize from bytes
class Record {
    public:
        std::vector<FieldValue> fieldValues;
        // Serialize to bytes buffer
        std::vector<char> serialize() const;
        // Deserialize from bytes
        static Record deserialize(const char* data, uint32_t length);

        std::string toString() const;
};

#endif //RECORD_H