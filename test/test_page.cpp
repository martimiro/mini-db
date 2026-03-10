#include "storage/page.h"
#include <iostream>
#include <cstring>

int passed = 0;
int failed = 0;

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
        std::cout << "FAIL [" << testName << "] — expected " << b << " got " << a << "\n";
        failed++;
    }
}

void assertThrows(auto f, const std::string& testName) {
    try {
        f();
        std::cout << "FAIL [" << testName << "] — expected exception\n";
        failed++;
    } catch (const std::exception&) {
        std::cout << "PASS [" << testName << "] (exception expected)\n";
        passed++;
    }
}

// =============================================================================
// TESTS
// =============================================================================
void testPageCreation() {
    Page page(1);
    assertEqual(page.getPageId(),   1, " Correct Page ID");
    assertEqual(page.getNumSlots(), 0, "Empty page -> 0 slots");
    assertTrue(page.freeSpace() > 0,  "New page have free space");
    assertTrue(page.freeSpace() < PAGE_SIZE, "freeSpace < PAGE_SIZE");
}

void testInsertAndGet() {
    Page page(1);

    const char* record = "Hello world";
    uint32_t len = strlen(record);
    uint32_t slot = page.insertRecord(record, len);

    assertEqual(slot, 0, "First slot is  0");
    assertEqual(page.getNumSlots(), 1, "numSlots is 1 after insert");

    uint32_t outLen = 0;
    const char* result = page.getRecord(0, outLen);
    assertEqual(outLen, len, "Lenght of register correct");
    assertTrue(std::memcmp(result, record, len) == 0, "Content of register correct");
}

void testMultipleInserts() {
    Page page(1);

    const char* r0 = "register zero";
    const char* r1 = "register one";
    const char* r2 = "register two";

    uint32_t s0 = page.insertRecord(r0, strlen(r0));
    uint32_t s1 = page.insertRecord(r1, strlen(r1));
    uint32_t s2 = page.insertRecord(r2, strlen(r2));

    assertEqual(s0, 0, "Slot 0 correct");
    assertEqual(s1, 1, "Slot 1 correct");
    assertEqual(s2, 2, "Slot 2 correct");
    assertEqual(page.getNumSlots(), 3, "3 slots after 3 inserts");

    uint32_t len;
    assertTrue(std::memcmp(page.getRecord(0, len), r0, strlen(r0)) == 0, "r0 correct");
    assertTrue(std::memcmp(page.getRecord(1, len), r1, strlen(r1)) == 0, "r1 correct");
    assertTrue(std::memcmp(page.getRecord(2, len), r2, strlen(r2)) == 0, "r2 correct");
}

void testFreeSpaceDecreases() {
    Page page(1);
    uint32_t spaceBefore = page.freeSpace();

    const char* record = "data";
    page.insertRecord(record, strlen(record));

    uint32_t spaceAfter = page.freeSpace();
    assertTrue(spaceAfter < spaceBefore, "freeSpace decrece tras insert");
}

void testDeleteRecord() {
    Page page(1);

    const char* record = "errase_me";
    uint32_t slot = page.insertRecord(record, strlen(record));

    page.deleteRecord(slot);

    uint32_t len;
    assertThrows([&]() {
        page.getRecord(slot, len);
    }, "getRecord in slot errase throw exception");
}

void testOutOfRangeSlot() {
    Page page(1);
    uint32_t len;
    assertThrows([&]() {
        page.getRecord(99, len);
    }, "getRecord slot inexistent throw exception");
}

void testDeleteOutOfRange() {
    Page page(1);
    assertThrows([&]() {
        page.deleteRecord(99);
    }, "deleteRecord slot inexistent throw exception");
}

void testPageFull() {
    Page page(1);

    // Insert register until complete space
    char record[100];
    std::memset(record, 'A', sizeof(record));

    int insertCount = 0;
    try {
        while (true) {
            page.insertRecord(record, sizeof(record));
            insertCount++;
        }
    } catch (const std::exception&) {}

    assertTrue(insertCount > 0, "Insert register before complete");
    assertTrue(page.freeSpace() < 100 + SLOT_SIZE, "Page full — freeSpace < record+slot");

    assertThrows([&]() {
        page.insertRecord(record, sizeof(record));
    }, "Insert in full page throw exception");
}

void testRawDataHeader() {
    // Verify page id survives a raw buffer round-trip
    Page page(42);
    const char* raw = page.rawData();

    uint32_t pageIdFromRaw;
    std::memcpy(&pageIdFromRaw, raw, sizeof(uint32_t));
    assertEqual(pageIdFromRaw, 42, "pageId correct in buffer raw");
}

// MAIN
int main() {
    std::cout << "TESTS DE PAGE\n";
    std::cout << "\n";

    std::cout << "Creation\n";
    testPageCreation();

    std::cout << "\nInsert and Get\n";
    testInsertAndGet();

    std::cout << "\nMultiples inserts\n";
    testMultipleInserts();

    std::cout << "\nFree spacen";
    testFreeSpaceDecreases();

    std::cout << "\nDelete\n";
    testDeleteRecord();

    std::cout << "\nOut of Range\n";
    testOutOfRangeSlot();
    testDeleteOutOfRange();

    std::cout << "\nFull Page\n";
    testPageFull();

    std::cout << "\nRaw Buffer\n";
    testRawDataHeader();

    std::cout << "  Results: " << passed << " passed, "<< failed << " failed\n";

    return failed == 0 ? 0 : 1;
}