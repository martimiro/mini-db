#include "transaction/wal.h"
#include <stdexcept>
#include <cstring>

// Constructor
WAL::WAL(const std::string& logFile) : logFile_(logFile) {
    writer_.open(logFile, std::ios::app | std::ios::binary);
    if (!writer_.is_open()) {
        throw std::runtime_error("WAL: cannot open log file: " + logFile);
    }
}

WAL::~WAL() {
    if (writer_.is_open()) {
        writer_.flush();
        writer_.close();
    }
}

// Write helpers
void WAL::writeString(const std::string& string) {
    uint32_t len = string.size();
    writer_.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
    writer_.write(string.data(), len);
}

void WAL::writeRecord(const Record& record) {
    auto data = record.serialize();
    uint32_t len = data.size();
    writer_.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
    writer_.write(data.data(), len);
}


// Append
void WAL::append(const LogRecord& record) {
    writer_.write(reinterpret_cast<const char*>(&record.transactionId), sizeof(uint32_t));

    uint8_t type = static_cast<uint8_t>(record.logType);
    writer_.write(reinterpret_cast<const char*>(&type), sizeof(uint8_t));

    writeString(record.tableName);

    writer_.write(reinterpret_cast<const char*>(&record.recordId.pageId), sizeof(uint32_t));
    writer_.write(reinterpret_cast<const char*>(&record.recordId.slotId), sizeof(uint32_t));

    writeRecord(record.oldRecord);
    writeRecord(record.newRecord);

    writer_.flush();
}

// Read Helpers
std::string WAL::readString(std::ifstream& f) {
    uint32_t len;
    f.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
    std::string s(len, '\0');
    f.read(s.data(), len);
    return s;
}

Record WAL::readRecord(std::ifstream& f) {
    uint32_t len;
    f.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
    std::vector<char> data(len);
    f.read(data.data(), len);
    if (len == 0) return Record{};
    return Record::deserialize(data.data(), len);
}

// Read all
std::vector<LogRecord> WAL::readAll() {
    std::vector<LogRecord> records;
    std::ifstream reader(logFile_, std::ios::binary);
    if (!reader.is_open()) return records;

    while (reader.peek() != EOF) {
        LogRecord rec;

        reader.read(reinterpret_cast<char*>(&rec.transactionId), sizeof(uint32_t));
        if (reader.eof()) break;

        uint8_t type;
        reader.read(reinterpret_cast<char*>(&type), sizeof(uint8_t));
        rec.logType = static_cast<LogType>(type);

        rec.tableName  = readString(reader);
        reader.read(reinterpret_cast<char*>(&rec.recordId.pageId), sizeof(uint32_t));
        reader.read(reinterpret_cast<char*>(&rec.recordId.slotId), sizeof(uint32_t));
        rec.oldRecord  = readRecord(reader);
        rec.newRecord  = readRecord(reader);

        records.push_back(std::move(rec));
    }

    return records;
}

// Flush
void WAL::flush() {
    writer_.flush();
}
// Truncate
void WAL::truncate() {
    writer_.close();
    writer_.open(logFile_, std::ios::trunc | std::ios::binary);
}