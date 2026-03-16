#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include "index/btree.h"
#include <string>
#include <unordered_map>
#include <memory>

// Index Info
struct IndexInfo {
    std::string tableName;
    std::string columnName;
    std::string indexName;
    int columnIndex;
};

// Index Manager
class IndexManager {
    private:
        // BPlusTree
        std::unordered_map<std::string, std::unique_ptr<BPlusTree>> indexes_;
        std::unordered_map<std::string, IndexInfo> indexInfos_;

        std::string indexKey(const std::string& tableName, const std::string& columnName) const {
            return tableName + "_" + columnName;
        }

    public:
        // Create new index
        void createIndex(const std::string& tableName, const std::string& columnName, const std::string& indexName,
            int columnIndex);
        // Check if index exisists for a column
        bool hasIndex(const std::string& tableName, const std::string& columnName) const;
        // Get B+Tree for a column
        BPlusTree& getIndex(const std::string& tableName, const std::string& columnName);
        // Get index info
        const IndexInfo& getIndexInfo(const std::string& tableName, const std::string& columnName) const;
        // Insert a key into the index
        void insertKey(const std::string& tableName, const std::string& columnName, BTreeKey key, RecordId recordId);
        // Remove a key
        void removeKey(const std::string& tableName, const std::string& columnName, BTreeKey key);
};
#endif //INDEX_MANAGER_H