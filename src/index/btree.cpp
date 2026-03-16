#include "index/btree.h"
#include <algorithm>
#include <iostream>

static constexpr int MAX_KEYS = 2 * BTREE_ORDER - 1; // 7
static constexpr int MIN_KEYS = BTREE_ORDER - 1; // 3

// Constructor
BPlusTree::BPlusTree() {
    size_  = 0;
    root_ = std::make_unique<BTreeNode>(true);
}

// Find leaf
BTreeNode* BPlusTree::findLeaf(BTreeKey key) {
    BTreeNode* node = root_.get();
    while (!node->isLeaf) {
        int i = 0;
        while (i < (int)node->keys.size() && key >= node->keys[i]) {
            i++;
        }
        node = node->children[i].get();
    }
    return node;
}

// Search
std::optional<RecordId> BPlusTree::search(BTreeKey key) {
    BTreeNode* leaf = findLeaf(key);
    for (size_t i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            return leaf->values[i];
        }
    }
    return std::nullopt;
}

// Range Search
std::vector<RecordId> BPlusTree::rangeSearch(BTreeKey low, BTreeKey high) {
    std::vector<RecordId> results;
    BTreeNode* leaf = findLeaf(low);

    while (leaf != nullptr) {
        for (size_t i = 0; i < leaf->keys.size(); i++) {
            if (leaf->keys[i] > high) {
                return results;
            }

            if (leaf->keys[i] >= low) {
                results.push_back(leaf->values[i]);
            }
        }
        leaf = leaf->next;
    }
    return results;
}

// Insert
void BPlusTree::insert(BTreeKey key, RecordId recordId) {
    auto splitResult = insertInto(root_.get(), key, recordId);

    if (splitResult.has_value()) {
        // Root split
        auto newRoot = std::make_unique<BTreeNode>(false);
        newRoot -> keys.push_back(splitResult->promoteKey);
        newRoot -> children.push_back(std::move(root_));
        newRoot -> children.push_back(std::move(splitResult->newNode));
        root_ = std::move(newRoot);
    }
    size_++;
}

// Insert into
std::optional<SplitResult> BPlusTree::insertInto(BTreeNode* node, BTreeKey key, RecordId recordId) {
    if (node->isLeaf) {
        auto iter = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        int index = iter - node->keys.begin();
        node -> keys.insert(iter, key);
        node -> values.insert(node->values.begin() + index, recordId);

        if (int(node->keys.size()) > MAX_KEYS) {
            return splitLeaf(node, key, recordId);
        }
        return std::nullopt;
    }

    int i = 0;
    while (i < node->keys.size() && key >= node->keys[i]) {
        i++;
    }

    auto splitResult = insertInto(node->children[i].get(), key, recordId);

    if (!splitResult.has_value()) {
        return std::nullopt;
    }

    // Child split
    node->keys.insert(node->keys.begin() + i, splitResult->promoteKey);
    node->children.insert(node->children.begin() + i + 1, std::move(splitResult->newNode));

    if ((int(node->keys.size()) > MAX_KEYS)) {
        return splitInternal(node, splitResult->promoteKey, std::move(node->children[i + 1]));
    }
    return std::nullopt;
}

// Split leaf
SplitResult BPlusTree::splitLeaf(BTreeNode *node, BTreeKey key, RecordId recordId) {
    int mid = (int)node->keys.size() / 2;
    auto newLeaf = std::make_unique<BTreeNode>(true);

    // Move right half to new leaf
    newLeaf -> keys.assign(node->keys.begin() + mid, node->keys.end());
    newLeaf -> values.assign(node->values.begin() + mid, node->values.end());

    node->keys.resize(mid);
    node->values.resize(mid);

    // Updated linked list
    newLeaf->next = node->next;
    node->next = newLeaf.get();

    BTreeKey promoted = newLeaf -> keys[0];
    return SplitResult{promoted, std::move(newLeaf)};
}

// Split internal
SplitResult BPlusTree::splitInternal(BTreeNode *node, BTreeKey promotedKey, std::unique_ptr<BTreeNode> newChild) {
    int mid = (int)node->keys.size() / 2;
    BTreeKey promoted = node->keys[mid];

    auto newNode = std::make_unique<BTreeNode>(false);

    // Move right half to new node
    newNode -> keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newNode -> children.assign(std::make_move_iterator(node->children.begin() + mid + 1),
        std::make_move_iterator(node->children.end()));

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    return SplitResult{promoted, std::move(newNode)};
}

// Remove
void BPlusTree::remove(BTreeKey key) {
    BTreeNode* leaf = findLeaf(key);
    for (size_t i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            leaf->keys.erase(leaf->keys.begin() + i);
            leaf->values.erase(leaf->values.begin() + i);
            size_--;
            return;
        }
    }
    throw std::runtime_error("Key not found");
}

// Print
void BPlusTree::print() const {
    printNode(root_.get(), 0);
}

void BPlusTree::printNode(const BTreeNode* node, int depth)  const {
    std::string indent(depth * 2, ' ');

    if (node->isLeaf) {
        std::cout << indent << "LEAF";
        for (size_t i = 0; i < node->keys.size(); i++) {
            std::cout << node->keys[i];
            if (i < node->keys.size() - 1) {
                std::cout << indent << ", ";
            }
        }
        std::cout << std::endl;
    } else {
        std::cout << indent << "NODE";
        for (size_t i = 0; i < node->keys.size(); i++) {
            std::cout << node->keys[i];
            if (i < node->keys.size() - 1) {
                std::cout << indent << ", ";
            }
        }
        std::cout << std::endl;
        for (const auto& child : node->children) {
            printNode(child.get(), depth + 1);
        }
    }
}