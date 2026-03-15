#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include "buffer_pool.h"
#include "disk_manager.h"
#include "record.h"
#include <string>

// Record ID
// Identifies a record by page and slot
struct RecordId {
    uint32_t pageId;
    uint32_t slotId;

    bool operator==(const RecordId &other) const {
        return pageId == other.pageId && slotId == other.slotId;
    }
};

// Heap File
// Stores collection of pages in disk -> provides, insert, get and delete
class HeapFile {
    private:
        DiskManager diskManager_;
        BufferPool bufferPool_;

        // Find a page with enough space for DataSize bytes
        uint32_t findOrCreatePage(uint32_t dataSize);
    public:
        HeapFile(const std::string& fileName, uint32_t bufferPoolSize = 16);
        ~HeapFile();

        // Insert record
        RecordId insertRecord(const Record& record);
        // Get a record
        Record getRecord(const RecordId& recordId);
        // Delete a record
        void deleteRecord(const RecordId& recordId);
        // Total number of pages
        uint32_t numPages() const;
        BufferPool& getBufferPool() { return bufferPool_; }

};

#endif //HEAP_FILE_H