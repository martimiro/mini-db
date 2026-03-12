// Cache in memory page
// We use LRU -> we quick last used page

#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include "storage/page.h"
#include "storage/disk_manager.h"
#include <unordered_map>
#include <list>
#include <memory>
#include <stdexcept>

// Frame
// Slot in the buffer pool that holds one page
struct Frame {
    std::unique_ptr<Page> page;
    // True if page has modified and have to be written to disk
    bool dirty;
    // number if active users
    int pinCount;

    Frame() {
        page = nullptr;
        dirty = false;
        pinCount = 0;
    }
};

class BufferPool {
    private:
        DiskManager& diskManager_;
        uint32_t poolSize_;

        // frameId -> Frame
        std::vector<Frame> frames_;
        // pageId -> frameId
        std::unordered_map<uint32_t, uint32_t> pageTable_;
        // LRU list
        std::list<uint32_t> lruList_;
        // FrameID -> iterator
        std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lruMap_;
        // Find a free frame o evict LRU
        uint32_t getFrame();
        // Move frame to front LRU list
        void touchFrame(uint32_t frameId);
        // Evict LRU frame
        uint32_t evictFrame();

    public:
        // poolSize -> max number of pages in memomry at once
        BufferPool(DiskManager& diskManager, uint32_t poolSize);
        // Fetch a page into memory
        Page* fetchPage(uint32_t pageId);
        // Allocate new page on disk and load into the pool
        Page* newPage(uint32_t& pageIdOut);
        // Unpin a page -> allow to be evicted
        // dirty = true if page was evicted
        void unpinPage(uint32_t pageId, bool dirty);
        // Force to write a page to disk immediately
        void flushPage(uint32_t pageId);
        // Write all pages to disk
        void flushAll();
        uint32_t poolSize() const {
            return poolSize_;
        }
};

#endif //BUFFER_PAGE_H