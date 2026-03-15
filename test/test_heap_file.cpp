#include "storage/heap_file.h"
#include "storage/record.h"
#include <iostream>
#include <filesystem>

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

void testInsertAndGet() {
    const std::string path = "/tmp/test_heap_insert.db";
    removeFile(path);

    HeapFile hf(path);

    Record r;
    r.fieldValues.push_back(FieldValue::makeInt(1));
    r.fieldValues.push_back(FieldValue::makeText("Mary"));
    r.fieldValues.push_back(FieldValue::makeInt(25));

    RecordId rid = hf.insertRecord(r);
    Record result = hf.getRecord(rid);

    assertEqual(result.fieldValues.size(), 3, "Record have 3 camps");
    assertTrue(result.fieldValues[0].type == FieldType::INT,  "Camp 0 is INT");
    assertTrue(result.fieldValues[1].type == FieldType::TEXT, "Camp 1 is TEXT");
    assertTrue(result.fieldValues[2].type == FieldType::INT,  "Camp 2 is INT");
    assertTrue(result.fieldValues[0].intValue == 1,           "Camp 0 correct value");
    assertTrue(result.fieldValues[1].textVale == "Mary",       "Camp 1 correct value");
    assertTrue(result.fieldValues[2].intValue == 25,          "Camp 2 correct value");

    removeFile(path);
}

void testMultipleInserts() {
    const std::string path = "/tmp/test_heap_multi.db";
    removeFile(path);

    HeapFile hf(path);

    std::vector<RecordId> rids;
    for (int i = 0; i < 5; i++) {
        Record r;
        r.fieldValues.push_back(FieldValue::makeInt(i));
        r.fieldValues.push_back(FieldValue::makeText("name" + std::to_string(i)));
        rids.push_back(hf.insertRecord(r));
    }

    for (int i = 0; i < 5; i++) {
        Record result = hf.getRecord(rids[i]);
        assertTrue(result.fieldValues[0].intValue == i,
                   "Record " + std::to_string(i) + "correct id");
        assertTrue(result.fieldValues[1].textVale == "name" + std::to_string(i),
                   "Record " + std::to_string(i) + " correct name");
    }

    removeFile(path);
}

void testDelete() {
    const std::string path = "/tmp/test_heap_delete.db";
    removeFile(path);

    HeapFile hf(path);

    Record r;
    r.fieldValues.push_back(FieldValue::makeInt(1));
    r.fieldValues.push_back(FieldValue::makeText("delete me"));
    RecordId rid = hf.insertRecord(r);

    hf.deleteRecord(rid);

    assertThrows([&]() {
        hf.getRecord(rid);
    }, "getRecord is erased slot throw exception");

    removeFile(path);
}

void testPersistence() {
    const std::string path = "/tmp/test_heap_persist.db";
    removeFile(path);

    RecordId rid;

    {
        HeapFile hf(path);
        Record r;
        r.fieldValues.push_back(FieldValue::makeInt(42));
        r.fieldValues.push_back(FieldValue::makeText("persistence"));
        rid = hf.insertRecord(r);
    }

    {
        HeapFile hf(path);
        Record result = hf.getRecord(rid);
        assertTrue(result.fieldValues[0].intValue == 42,
                   "INT persist after reopen");
        assertTrue(result.fieldValues[1].textVale == "persistence",
                   "TEXT persist after reopen");
    }

    removeFile(path);
}

void testNumPages() {
    const std::string path = "/tmp/test_heap_pages.db";
    removeFile(path);

    HeapFile hf(path);
    assertEqual(hf.numPages(), 0, "HeapFile new have 0 pages");

    Record r;
    r.fieldValues.push_back(FieldValue::makeInt(1));
    hf.insertRecord(r);

    assertTrue(hf.numPages() >= 1, "HeapFile have at least 1 page after insert");

    removeFile(path);
}

int main() {
    std::cout << "TESTS DE HEAP FILE\n";

    std::cout << "Insert and Get\n";
    testInsertAndGet();

    std::cout << "\nMultiples inserts\n";
    testMultipleInserts();

    std::cout << "\nDelete\n";
    testDelete();

    std::cout << "\nPersistence\n";
    testPersistence();

    std::cout << "\nNum pages\n";
    testNumPages();

    std::cout << "  Results: " << passed << " passed, "<< failed << " failed\n";

    return failed == 0 ? 0 : 1;
}