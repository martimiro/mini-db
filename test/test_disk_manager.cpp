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


// Tests
void testCreateFile() {
    const std::string path = "/tmp/test_create.db";
    removeFile(path);

    DiskManager dm(path);
    assertEqual(dm.getNumPages(), 0, "New archive have 0 pages");
    assertTrue(std::filesystem::exists(path), ".db archives created on disk");

    removeFile(path);
}

void testAllocateAndWrite() {
    const std::string path = "/tmp/test_alloc.db";
    removeFile(path);

    DiskManager dm(path);
    uint32_t id = dm.allocatePage();
    assertEqual(id, 0, "First page have id 0");
    assertEqual(dm.getNumPages(), 1, "numPages is 1 after allocate");

    uint32_t id2 = dm.allocatePage();
    assertEqual(id2, 1, "Second page have id 1");
    assertEqual(dm.getNumPages(), 2, "numPages is 2 after alllocate");

    removeFile(path);
}

void testWriteAndRead() {
    const std::string path = "/tmp/test_write_read.db";
    removeFile(path);

    DiskManager dm(path);
    uint32_t pageId = dm.allocatePage();

    Page page(pageId);
    const char* record = "Hello world";
    page.insertRecord(record, strlen(record));
    dm.writePage(pageId, page.rawData());

    char buffer[PAGE_SIZE];
    dm.readPage(pageId, buffer);

    Page loaded = Page::fromRawData(buffer);
    uint32_t len = 0;
    const char* result = loaded.getRecord(0, len);
    assertTrue(std::memcmp(result, record, strlen(record)) == 0,
               "Content of reg correct afeter round-trip disk");
    assertEqual(loaded.getPageId(), pageId, "pageId correct after read");

    removeFile(path);
}

void testPersistence() {
    const std::string path = "/tmp/test_persist.db";
    removeFile(path);

    {
        DiskManager dm(path);
        uint32_t pageId = dm.allocatePage();
        Page page(pageId);
        const char* record = "persitance";
        page.insertRecord(record, strlen(record));
        dm.writePage(pageId, page.rawData());
    }

    {
        DiskManager dm(path);
        assertEqual(dm.getNumPages(), 1, "Page persist after reopening the file");

        char buffer[PAGE_SIZE];
        dm.readPage(0, buffer);

        Page loaded = Page::fromRawData(buffer);
        uint32_t len = 0;
        const char* result = loaded.getRecord(0, len);
        assertTrue(std::memcmp(result, "persitance", 12) == 0,
                   "Reg persist after reopening file");
    }

    removeFile(path);
}

void testReadInvalidPage() {
    const std::string path = "/tmp/test_invalid.db";
    removeFile(path);

    DiskManager dm(path);
    char buffer[PAGE_SIZE];
    assertThrows([&]() {
        dm.readPage(99, buffer);
    }, "Read inexsitent page throw exception");

    removeFile(path);
}

void testMultiplePages() {
    const std::string path = "/tmp/test_multi.db";
    removeFile(path);

    DiskManager dm(path);

    for (uint32_t i = 0; i < 5; i++) {
        uint32_t id = dm.allocatePage();
        Page page(id);
        std::string record = "page " + std::to_string(i);
        page.insertRecord(record.c_str(), record.size());
        dm.writePage(id, page.rawData());
    }

    assertEqual(dm.getNumPages(), 5, "5 pages written correctly");

    for (uint32_t i = 0; i < 5; i++) {
        char buffer[PAGE_SIZE];
        dm.readPage(i, buffer);
        Page loaded = Page::fromRawData(buffer);
        uint32_t len = 0;
        const char* result = loaded.getRecord(0, len);
        std::string expected = "page " + std::to_string(i);
        assertTrue(std::memcmp(result, expected.c_str(), expected.size()) == 0,
                   "Content page " + std::to_string(i) + " correct");
    }

    removeFile(path);
}

// Main
int main() {
    std::cout << " TESTS DE DISK MANAGER\n";

    std::cout << "Create file\n";
    testCreateFile();

    std::cout << "\nAllocate pages\n";
    testAllocateAndWrite();

    std::cout << "\nWrite and read\n";
    testWriteAndRead();

    std::cout << "\nPersitancen";
    testPersistence();

    std::cout << "\nInvalid page\n";
    testReadInvalidPage();

    std::cout << "\nMultiple pages\n";
    testMultiplePages();

    std::cout << "  Results: " << passed << " passed, "
              << failed << " failded\n";

    return failed == 0 ? 0 : 1;
}