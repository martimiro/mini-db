#include "storage/page.h"
#include <iostream>
#include <cstring>

// Constructor
Page::Page(uint32_t pageId) {
    // Zero out the entire buffer
    std::memset(data_, 0, PAGE_SIZE);

    // Initializate headers
    header_.pageId = pageId;
    header_.numSlots = 0;
    header_.freeSpaceOffset = HEADER_SIZE;
    header_.flags = 0;

    writeHeader();
}

// Header Syncronitation
void Page::writeHeader() {
    std::memcpy(data_, &header_, sizeof(PageHeader));
}

void Page::readHeader() {
    std::memcpy(&header_, data_, sizeof(PageHeader));
}

// Slot acces
// data_[HEADER_SIZE + slotIndex * SLOT_SIZE]
Slot Page::getSlot(uint32_t slotIndex) const {
    Slot slot;
    uint32_t slotOffset = HEADER_SIZE + slotIndex * SLOT_SIZE;
    std::memcpy(&slot, data_ + slotOffset, sizeof(Slot));
    return slot;
}

void Page::setSlot(uint32_t slotIndex, Slot slot) {
    uint32_t slotOffset = HEADER_SIZE + slotIndex * SLOT_SIZE;
    std::memcpy(&data_[slotOffset], &slot, sizeof(Slot));
}

// Free space
uint32_t Page::dataEnd() const {
    // Find the lowets data offset among all valid slots
    uint32_t end = PAGE_SIZE;
    for (uint32_t i = 0; i < header_.numSlots; ++i) {
        Slot slot = getSlot(i);
        if (slot.offset != INVALID_OFFSET) {
            end = std::min(end, slot.offset);
        }
    }
    return end;
}

uint32_t Page::freeSpace() const {
    // gap between  end of slot array and start of data
    uint32_t slotArrayEnd = HEADER_SIZE + header_.numSlots * SLOT_SIZE;
    uint32_t dataStart = dataEnd();

    if (dataStart <= slotArrayEnd) {
        return 0;
    }

    return slotArrayEnd - dataStart;
}

// Insert Record
uint32_t Page::insertRecord(const char *data, uint32_t length) {
    // Check if there is suficient space
    uint32_t needed = length + SLOT_SIZE;
    if (freeSpace() < needed) {
        throw std::runtime_error("Page " + std::to_string(header_.pageId) + " is full");
    }

    // Place data at the top of the data section
    uint32_t recordOffset = dataEnd() - length;
    std::memcpy(data_ + recordOffset, data, length);

    uint32_t slotIndex = header_.numSlots;
    setSlot(slotIndex, {recordOffset, length});

    header_.numSlots++;
    header_.freeSpaceOffset += HEADER_SIZE + slotIndex * SLOT_SIZE;
    writeHeader();

    return slotIndex;
}

// Headers
const char* Page::getRecord(uint32_t slotIndex, uint32_t& length) const {
    if (slotIndex >= header_.numSlots) {
        throw std::runtime_error("Slot index " + std::to_string(slotIndex) + " is out of range");
    }

    Slot slot = getSlot(slotIndex);
    if (slot.offset == INVALID_OFFSET) {
        throw std::runtime_error("Slot " + std::to_string(slotIndex) + " has been removed");
    }

    length = slot.length;
    return data_ + slot.offset;
}

// Delete record
// Mark slot as invalid
void Page::deleteRecord(uint32_t slotIndex) {
    if (slotIndex >= header_.numSlots) {
        throw std::runtime_error("Slot index" + std::to_string(slotIndex) + " is out of range");
    }

    setSlot(slotIndex, {INVALID_OFFSET, 0});
}
