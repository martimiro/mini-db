#include "storage/buffer_pool.h"
#include <stdexcept>

// Constructor
BufferPool::BufferPool(DiskManager& diskManager, uint32_t poolSize)
    : diskManager_(diskManager), poolSize_(poolSize) {
    frames_.resize(poolSize);
    for (uint32_t i = 0; i < poolSize; i++) {
        lruList_.push_back(i);
        lruMap_[i] = std::prev(lruList_.end());
    }
}

// Touch frame
void BufferPool::touchFrame(uint32_t frameId) {
    lruList_.erase(lruMap_[frameId]);
    lruList_.push_front(frameId);
    lruMap_[frameId] = lruList_.begin();
}

// Evict frame
uint32_t BufferPool::evictFrame() {
    // Traverse from back LRU to find and unpinned frame
    for (auto iter = lruList_.begin(); iter != lruList_.end(); ++iter) {
        uint32_t frameId = *iter;
        Frame& frame = frames_[frameId];

        // Skip pinned frames
        if (frame.pinCount > 0) {
            continue;
        }

        // Write to disk if written
        if (frame.dirty && frame.page != nullptr) {
            diskManager_.writePage(frame.page->getPageId(), frame.page->rawData());
        }

        // Remove from page table
        if (frame.page != nullptr) {
            pageTable_.erase(frame.page->getPageId());
        }

        // Reset frame
        frame.page = nullptr;
        frame.dirty = false;
        frame.pinCount = 0;

        return frameId;
    }

    throw std::runtime_error("BufferPool::evictFrame() called when not in use");
}

// Get frame
uint32_t BufferPool::getFrame() {
    // Look for an empty frame first
    for (uint32_t i = 0; i < poolSize_; i++) {
        if (frames_[i].page == nullptr) {
            return i;
        }
    }

    // No free frame
    return evictFrame();
}

// Fetch page
Page *BufferPool::fetchPage(uint32_t pageId) {
    // Already in pool?
    auto iter = pageTable_.find(pageId);
    if (iter != pageTable_.end()) {
        uint32_t frameId = iter->second;
        frames_[frameId].pinCount++;
        touchFrame(frameId);
        return frames_[frameId].page.get();
    }

    uint32_t frameId = getFrame();
    Frame& frame = frames_[frameId];

    char buffer[PAGE_SIZE];
    diskManager_.readPage(pageId, buffer);

    frame.page = std::make_unique<Page>(Page::fromRawData(buffer));
    frame.dirty = false;
    frame.pinCount = 1;

    pageTable_[pageId] = frameId;
    touchFrame(frameId);

    return frame.page.get();
}

// New page
void BufferPool::flushPage(uint32_t pageId) {
    auto iter = pageTable_.find(pageId);
    if (iter == pageTable_.end()) {
        return;
    }

    Frame& frame = frames_[iter->second];
    if (frame.page != nullptr && frame.dirty) {
        diskManager_.writePage(pageId, frame.page->rawData());
        frame.dirty = false;
    }
}

// Flush all
void BufferPool::flushAll() {
    for (auto& [pageId, frameId] : pageTable_) {
        Frame& frame = frames_[frameId];
        if (frame.page != nullptr && frame.dirty) {
            diskManager_.writePage(pageId, frame.page->rawData());
            frame.dirty = false;
        }
    }
}

// New page
Page *BufferPool::newPage(uint32_t &pageIdOut) {
    uint32_t pageId = diskManager_.allocatePage();
    pageIdOut = pageId;
    uint32_t frameId = getFrame();
    Frame& frame = frames_[frameId];

    frame.page = std::make_unique<Page>(pageId);
    frame.dirty = true;
    frame.pinCount = 1;

    pageTable_[pageId] = frameId;
    touchFrame(frameId);

    return frame.page.get();
}

// Unpin page
void BufferPool::unpinPage(uint32_t pageId, bool dirty) {
    auto iter = pageTable_.find(pageId);
    if (iter == pageTable_.end()) {
        throw std::runtime_error("BufferPool::unpinPage() called when not in use");
    }

    Frame& frame = frames_[iter->second];
    if (frame.pinCount <= 0) {
        throw std::runtime_error("BufferPool::unpinPage() called when not in use");
    }

    frame.pinCount--;
    if (dirty) {
        frame.dirty = true;
    }
}