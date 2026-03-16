// Write-Ahead Log

#ifndef WAL_H
#define WAL_H

#include "storage/record.h"
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

#include "storage/heap_file.h"

// Log record types
enum class LogType : uint8_t {
  BEGIN = 0,
  INSERT = 1,
  DELETE = 2,
  UPDATE = 3,
  COMMIT = 4,
  ROLLBACK = 5,
};

// Log record -> one entry in the WAL file
struct LogRecord {
  uint32_t transactionId;
  // Type of operation
  LogType logType;
  std::string tableName;
  // Afected record
  RecordId recordId;
  Record oldRecord;
  Record newRecord;
};

// WAL
class WAL {
  private:
    std::string logFile_;
    std::ofstream writer_;

    void writeString(const std::string & str);
    void writeRecord(const Record & record);
    std::string readString(std::ifstream& val);
    Record readRecord(std::ifstream& val);

  public:
    explicit WAL(const std::string& logFile);
    ~WAL();

    // Append log record
    void append(const LogRecord& record);
    // Read all log records
    std::vector<LogRecord> readAll();
    // Flush log to disk
    void flush();
    // Truncate log
    void truncate();
};
#endif //WAL_H