#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include "storage/page.h"
#include <string>
#include <fstream>

// Disk manager
// Read and write fixes-size pages to/from bin file
// EXxample -> [page 0: 4096 bytes][page 1: 4096 bytes][page 2: 4096 bytes]

// To find page N: seek to N * PAGE_SIZE

class DiskManager {
    private:
        std::string filename_;
        std::fstream file_;
        uint32_t numPages_;

    public:
        // Open or create a .db file
        explicit DiskManager(const std::string& filename);
        ~DiskManager();
        // Write a page to disk at position pageID * PAGE_SIZE
        void writePage(uint32_t pageId, const char* data);
        // Read a page from disk into buffer (must be PAGE_SIZE bytes)
        void readPage(uint32_t pageId, char* buffer);
        // Total number of pages in file
        uint32_t getNumPages();
        // Allocate new pages
        uint32_t allocatePage();

        const std::string fileName() const {
            return filename_;
        }

};

#endif //DISK_MANAGER_H