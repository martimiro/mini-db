#include "storage/heap_file.h"
#include <stdexcept>

// Constructor / Destructor
HeapFile::HeapFile(const std::string& filename, uint32_t bufferPoolSize)
    : diskManager_(filename), bufferPool_(diskManager_, bufferPoolSize) {}

HeapFile::~HeapFile() {
    bufferPool_.flushAll();
}

// Find or create page
uint32_t HeapFile::findOrCreatePage(uint32_t dataSize) {
    uint32_t needed = dataSize + SLOT_SIZE;

    // Scan existing pages for one with enough space
    for (uint32_t i = 0; i < diskManager_.getNumPages(); i++) {
        Page* page = bufferPool_.fetchPage(i);
        uint32_t free = page -> freeSpace();
        bufferPool_.unpinPage(i, false);

        if (free >=needed) {
            return i;
        }
    }

    uint32_t newPageId;
    Page* page = bufferPool_.newPage(newPageId);
    bufferPool_.unpinPage(newPageId, false);
    return newPageId;
}

// Insert record
RecordId HeapFile::insertRecord(const Record &record) {
    auto data = record.serialize();
    uint32_t dataSize = static_cast<uint32_t>(data.size());
    uint32_t pageId = findOrCreatePage(dataSize);
    Page* page = bufferPool_.fetchPage(pageId);
    uint32_t slotId = page -> insertRecord(data.data(), dataSize);
    bufferPool_.unpinPage(pageId, true);
    return RecordId{pageId, slotId};
}

// Get record
Record HeapFile::getRecord(const RecordId& recordId) {
    Page* page = bufferPool_.fetchPage(recordId.pageId);
    uint32_t length = 0;
    const char* data = page->getRecord(recordId.slotId, length);
    Record record = Record::deserialize(data, length);
    bufferPool_.unpinPage(recordId.pageId, false);
    return record;
}

// Delete record
void HeapFile::deleteRecord(const RecordId& recordId) {
    Page* page = bufferPool_.fetchPage(recordId.pageId);
    page->deleteRecord(recordId.slotId);
    bufferPool_.unpinPage(recordId.pageId, true);
}

// Num pages
uint32_t HeapFile::numPages() const{
    return diskManager_.getNumPages();
}