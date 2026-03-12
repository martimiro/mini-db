#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/page.h"
#include <iostream>
#include <cstring>
#include <filesystem>

int passed = 0;
int failed = 0;

// Utils
void assertTrue(bool condition, const std::string& testName) {
    if (condition) {
        std::cout << "PASS [" << testName << "]\n";
        passed++;
    } else {
        std::cout << "FAIL [" << testName << "]\n";
        failed++;
    }
}

void assertEqual(uint32_t a, uint32_t b, const std::string& testName) {
    if (a == b) {
        std::cout << "PASS [" << testName << "]\n";
        passed++;
    } else {
        std::cout << "FAIL [" << testName << "] — expected "
                  << b << " got " << a << "\n";
        failed++;
    }
}

template<typename F>
void assertThrows(F f, const std::string& testName) {
    try {
        f();
        std::cout << "FAIL [" << testName << "] — expected exception\n";
        failed++;
    } catch (const std::exception&) {
        std::cout << "PASS [" << testName << "] (exception expected)\n";
        passed++;
    }
}

void removeFile(const std::string& path) {
    std::filesystem::remove(path);
}


// Test
void testNewPage() {
    const std::string path = "/tmp/test_bp_new.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 4);

    uint32_t pageId;
    Page* page = bp.newPage(pageId);

    assertTrue(page != nullptr, "newPage returns valid pointer");
    assertEqual(pageId, 0, "First page have id 0");

    bp.unpinPage(pageId, false);
    removeFile(path);
}

void testFetchPage() {
    const std::string path = "/tmp/test_bp_fetch.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 4);

    // Create and write a page
    uint32_t pageId;
    Page* page = bp.newPage(pageId);
    const char* record = "fetch test";
    page->insertRecord(record, strlen(record));
    bp.unpinPage(pageId, true);

    // Flush to disk
    bp.flushPage(pageId);

    // Fetch it back
    Page* fetched = bp.fetchPage(pageId);
    assertTrue(fetched != nullptr, "fetchPage returns valid pointer");

    uint32_t len = 0;
    const char* result = fetched->getRecord(0, len);
    assertTrue(std::memcmp(result, record, strlen(record)) == 0,
               "fetchPage returns valid content");

    bp.unpinPage(pageId, false);
    removeFile(path);
}

void testFetchSamePage() {
    const std::string path = "/tmp/test_bp_same.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 4);

    uint32_t pageId;
    Page* p1 = bp.newPage(pageId);
    Page* p2 = bp.fetchPage(pageId);

    // Both pointers should point to the same frame
    assertTrue(p1 == p2, "fetchPage of page colsed and returns same pointer");

    bp.unpinPage(pageId, false);
    bp.unpinPage(pageId, false);
    removeFile(path);
}

void testEviction() {
    const std::string path = "/tmp/test_bp_evict.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 2);  // Only to frames

    // Fill the pool
    uint32_t id0, id1;
    Page* p0 = bp.newPage(id0);
    const char* r0 = "page zero";
    p0->insertRecord(r0, strlen(r0));
    bp.unpinPage(id0, true);

    Page* p1 = bp.newPage(id1);
    const char* r1 = "page one";
    p1->insertRecord(r1, strlen(r1));
    bp.unpinPage(id1, true);

    // Allocate a third page — forces eviction of LRU (id0)
    uint32_t id2;
    Page* p2 = bp.newPage(id2);
    const char* r2 = "page two";
    p2->insertRecord(r2, strlen(r2));
    bp.unpinPage(id2, true);

    bp.flushAll();

    // Fetch id0 from disk — should have been evicted and written
    Page* reloaded = bp.fetchPage(id0);
    uint32_t len = 0;
    const char* result = reloaded->getRecord(0, len);
    assertTrue(std::memcmp(result, r0, strlen(r0)) == 0,
               "Page evicted persist in disk");

    bp.unpinPage(id0, false);
    removeFile(path);
}

void testDirtyFlag() {
    const std::string path = "/tmp/test_bp_dirty.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 4);

    uint32_t pageId;
    Page* page = bp.newPage(pageId);
    const char* record = "dirty test";
    page->insertRecord(record, strlen(record));

    // Unpin as dirty — should be written on eviction
    bp.unpinPage(pageId, true);
    bp.flushAll();

    // Reopen DiskManager to verify persistence
    DiskManager dm2(path);
    BufferPool bp2(dm2, 4);

    Page* loaded = bp2.fetchPage(pageId);
    uint32_t len = 0;
    const char* result = loaded->getRecord(0, len);
    assertTrue(std::memcmp(result, record, strlen(record)) == 0,
               "Página dirty persist after flushAll");

    bp2.unpinPage(pageId, false);
    removeFile(path);
}

void testAllPinned() {
    const std::string path = "/tmp/test_bp_pinned.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 2);  // frames

    // Pin both frames
    uint32_t id0, id1;
    bp.newPage(id0);  // pinned, not unpinned
    bp.newPage(id1);  // pinned, not unpinned

    // Try to allocate a third — all frames pinned, should throw
    assertThrows([&]() {
        uint32_t id2;
        bp.newPage(id2);
    }, "Pool full with full pages pinned throw exception");

    bp.unpinPage(id0, false);
    bp.unpinPage(id1, false);
    removeFile(path);
}

void testUnpinInvalid() {
    const std::string path = "/tmp/test_bp_unpin.db";
    removeFile(path);

    DiskManager dm(path);
    BufferPool bp(dm, 4);

    assertThrows([&]() {
        bp.unpinPage(99, false);
    }, "unpinPage of page not charged thow excpetion");

    removeFile(path);
}


// Main
int main() {
    std::cout << "TEST OF BUFFER POOL\n";
    std::cout << "New page\n";
    testNewPage();

    std::cout << "\nFetch page\n";
    testFetchPage();

    std::cout << "\nFetch misma página\n";
    testFetchSamePage();

    std::cout << "\nEviction LRU\n";
    testEviction();

    std::cout << "\nDirty flag\n";
    testDirtyFlag();

    std::cout << "\nAll pinned\n";
    testAllPinned();

    std::cout << "\nUnpin inválido\n";
    testUnpinInvalid();

    std::cout << "  Results: " << passed << " pased, "<< failed << " failed\n";

    return failed == 0 ? 0 : 1;
}