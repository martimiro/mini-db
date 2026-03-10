#ifndef PAGE_H
#define PAGE_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

// Constants
static constexpr uint32_t PAGE_SIZE = 4096;
static constexpr uint32_t HEADER_SIDE = 16;
static constexpr uint32_t SOLT_SIZE = 8;
static constexpr uint32_t INVALID_OFFSET = 0xFFFFFFFF;

// Slots
struct Slot {
    uint32_t offset;
    uint32_t length;
};

// Page Header
struct PageHeader {
    uint32_t pageId;
    uint32_t numSlots;
    uint32_t freeSpaceOffset;
    uint32_t flags;
};

// Page
class Page {
    private:
        // Raw 4KB of memory
        char data_[PAGE_SIZE];
        // in memory copy of header
        PageHeader header_;

        // Sync header from/to buffer
        void writeHeader();
        void readHeader();

        // Slot acces
        Slot getSlot(uint32_t slotIndex) const;
        void setSlot(uint32_t slotIndex, Slot slot);

        // Where the data section ends
        uint32_t dataEnd() const;

    public:
        // Creates a empty page with a given ID
        explicit Page(uint32_t pageId);

        // Insert a record
        uint32_t insertRecord(const char* data, uint32_t length);
        // Read a record by slot index
        const char* getRecord(uint32_t slotIndex, uint32_t& length) const;
        // Delete a record
        void deleteRecord(uint32_t slotIndex);
        // Free space remainig in bytes
        uint32_t freeSpace() const;

        // Getters
        uint32_t getPageId() const {
            return header_.pageId;
        }

        uint32_t getNumSlots() const {
            return header_.numSlots;
        }

        // Raw acces to underlying bugger (disk I/O)
        const char* rawData() const {
            return data_;
        }

        char* rawData() {
            return data_;
        }
};

#endif //PAGE_H