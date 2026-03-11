#include "storage/disk_manager.h"
#include <stdexcept>
#include <cstring>
#include <filesystem>

// Constructor
DiskManager::DiskManager(const std::string& filename) {
    filename_ = filename;
    numPages_ = 0;

    file_.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_.is_open()) {
        file_.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open file for reading");
        }
    }

    file_.seekg(0, std::ios::end);
    std::streamsize fileSize = file_.tellg();
    numPages_ = static_cast<uint32_t>(fileSize / PAGE_SIZE);
}

DiskManager::~DiskManager() {
    if (file_.is_open()) {
        file_.close();
    }
}

// Write page
void DiskManager::writePage(uint32_t pageId, const char* data) {
    std::streampos offset = static_cast<std::streampos>(pageId * PAGE_SIZE);

    file_.seekp(offset);
    if (file_.fail()) {
        throw std::runtime_error("Failed to write page to disk");
    }

    file_.write(data, PAGE_SIZE);
    if (file_.fail()) {
        throw std::runtime_error("Failed to write page to disk");
    }

    file_.flush();

    if (pageId >= numPages_) {
        numPages_ = pageId + 1;
    }
}

// Read page
void DiskManager::readPage(uint32_t pageId, char* data) {
    if (pageId >= numPages_) {
        throw std::runtime_error("Invalid page id");
    }

    std::streampos offset = static_cast<std::streampos>(pageId * PAGE_SIZE);
    file_.seekg(offset);
    if (file_.fail()) {
        throw std::runtime_error("Failed to read page from disk");
    }

    file_.read(data, PAGE_SIZE);
    if (file_.fail()) {
        throw std::runtime_error("Failed to read page from disk");
    }
}

// Disk pages
uint32_t DiskManager::getNumPages() {
    return numPages_;
}

// Allocate pages
uint32_t DiskManager::allocatePage() {
    uint32_t newPageId = numPages_++;

    // Write a zeroed page at the end
    char emptyData[PAGE_SIZE];
    std::memset(emptyData, 0, PAGE_SIZE);
    writePage(newPageId, emptyData);

    return newPageId;
}