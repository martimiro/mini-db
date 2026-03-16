// Without the BTree when we do SELECT we do a full table scan. With B+Tree we can find with O(log n) complexity
#ifndef BTREE_H
#define BTREE_H

#include "storage/record.h"
#include <vector>
#include <memory>
#include <optional>

#include "storage/heap_file.h"

// Constants
// Max keys per node = 2 * ORDER - 1
static constexpr int BTREE_ORDER = 4;

// Key type
// Only INT keys are sypported
using BTreeKey = int64_t;

// Node
struct BTreeNode {
    bool isLeaf;
    std::vector<BTreeKey> keys;
    // Subtree for keys
    std::vector<std::unique_ptr<BTreeNode>> children;
    // Leaf nodes
    std::vector<RecordId> values;
    // Pointer to the next Leaf
    BTreeNode* next;

    explicit BTreeNode (bool leaf) {
        isLeaf = leaf;
        next = nullptr;
    }
};

// Split result
struct SplitResult {
    BTreeKey promotedKey;
    std::unique_ptr<BTreeNode> newNode;
};

// B+Tree
class BPlusTree {
    private:
        std::unique_ptr<BTreeNode> root_;
        uint32_t size_;

        // Insert into subtree
        std::optional<SplitResult> insertInto(BTreeNode* node, BTreeKey key, RecordId recordId);
        // Split a leaf node
        SplitResult splitLeaf(BTreeNode* node, BTreeKey key, RecordId recordId);
        // Split internal node
        SplitResult splitInternal(BTreeNode* node, BTreeKey promotedKey, std::unique_ptr<BTreeNode> newChild);
        // Find leaf node for key
        BTreeNode* findLeaf(BTreeKey key);
        // Print helper
        void printNode(const BTreeNode* node, int depth) const;

    public:
        BPlusTree();

        // Insert a key value
        void insert(BTreeKey key, RecordId recordId);
        // Search for exact key
        std::optional<RecordId> search(BTreeKey key);
        // Range search
        std::vector<RecordId> rangeSearch(BTreeKey low, BTreeKey high);
        // Remve key
        void remove(BTreeKey key);
        // Number of entries
        uint32_t size() const {
            return size_;
        }
        // Print tree structure (debbuging)
        void print() const;

};
#endif //BTREE_H