#include "index/btree.h"
#include <iostream>

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

// Tests
void testInsertAndSearch() {
    BPlusTree tree;

    tree.insert(1, {0, 0});
    tree.insert(2, {0, 1});
    tree.insert(3, {0, 2});

    auto r1 = tree.search(1);
    auto r2 = tree.search(2);
    auto r3 = tree.search(3);

    assertTrue(r1.has_value(),          "Search key 1 found");
    assertTrue(r1->pageId == 0 && r1->slotId == 0, "Key 1 correct RecordId");
    assertTrue(r2.has_value(),          "Search key 2 found");
    assertTrue(r3.has_value(),          "Search key 3 found");
    assertTrue(!tree.search(99).has_value(), "Search missing key returns nullopt");
}

void testSize() {
    BPlusTree tree;

    assertEqual(tree.size(), 0, "Empty tree has size 0");
    tree.insert(1, {0, 0});
    assertEqual(tree.size(), 1, "Size 1 after insert");
    tree.insert(2, {0, 1});
    tree.insert(3, {0, 2});
    assertEqual(tree.size(), 3, "Size 3 after 3 inserts");
}

void testManyInserts() {
    BPlusTree tree;

    // Insert enough to trigger splits
    for (int i = 1; i <= 20; i++) {
        tree.insert(i, {0, (uint32_t)i});
    }

    assertEqual(tree.size(), 20, "Size 20 after 20 inserts");

    // All keys should be findable
    bool allFound = true;
    for (int i = 1; i <= 20; i++) {
        if (!tree.search(i).has_value()) {
            allFound = false;
            break;
        }
    }
    assertTrue(allFound, "All 20 keys found after splits");
}

void testInsertOutOfOrder() {
    BPlusTree tree;

    tree.insert(5, {0, 5});
    tree.insert(1, {0, 1});
    tree.insert(9, {0, 9});
    tree.insert(3, {0, 3});
    tree.insert(7, {0, 7});

    assertTrue(tree.search(1).has_value(), "Key 1 found after out-of-order insert");
    assertTrue(tree.search(3).has_value(), "Key 3 found after out-of-order insert");
    assertTrue(tree.search(5).has_value(), "Key 5 found after out-of-order insert");
    assertTrue(tree.search(7).has_value(), "Key 7 found after out-of-order insert");
    assertTrue(tree.search(9).has_value(), "Key 9 found after out-of-order insert");
}

void testRangeSearch() {
    BPlusTree tree;

    for (int i = 1; i <= 10; i++) {
        tree.insert(i, {0, (uint32_t)i});
    }

    auto results = tree.rangeSearch(3, 7);
    assertEqual(results.size(), 5, "Range [3,7] returns 5 results");
    assertTrue(results[0].slotId == 3, "Range first result is key 3");
    assertTrue(results[4].slotId == 7, "Range last result is key 7");

    auto all = tree.rangeSearch(1, 10);
    assertEqual(all.size(), 10, "Range [1,10] returns all 10 results");

    auto empty = tree.rangeSearch(11, 20);
    assertEqual(empty.size(), 0, "Range [11,20] returns 0 results");
}

void testRemove() {
    BPlusTree tree;

    tree.insert(1, {0, 0});
    tree.insert(2, {0, 1});
    tree.insert(3, {0, 2});

    tree.remove(2);
    assertEqual(tree.size(), 2, "Size 2 after remove");
    assertTrue(!tree.search(2).has_value(), "Removed key not found");
    assertTrue(tree.search(1).has_value(),  "Key 1 still found after remove");
    assertTrue(tree.search(3).has_value(),  "Key 3 still found after remove");
}

void testRemoveNotFound() {
    BPlusTree tree;
    tree.insert(1, {0, 0});

    assertThrows([&]() {
        tree.remove(99);
    }, "Remove missing key throws exception");
}

void testRangeAfterRemove() {
    BPlusTree tree;

    for (int i = 1; i <= 5; i++) {
        tree.insert(i, {0, (uint32_t)i});
    }

    tree.remove(3);
    auto results = tree.rangeSearch(1, 5);
    assertEqual(results.size(), 4, "Range [1,5] returns 4 after removing key 3");
}

// Main
int main() {
    std::cout << "TESTS BTREE\n\n";

    std::cout << "INSERT AND SEARCH\n";
    testInsertAndSearch();

    std::cout << "\nSIZE\n";
    testSize();

    std::cout << "\nMANY INSERTS (splits)\n";
    testManyInserts();

    std::cout << "\nOUT OF ORDER\n";
    testInsertOutOfOrder();

    std::cout << "\nRANGE SEARCH\n";
    testRangeSearch();

    std::cout << "\nREMOVE\n";
    testRemove();
    testRemoveNotFound();
    testRangeAfterRemove();

    std::cout << "\nResults: " << passed << " passed, "
              << failed << " failed\n";

    return failed == 0 ? 0 : 1;
}