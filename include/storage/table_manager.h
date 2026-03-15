// Connect the semantic catalogue with the disk files

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "storage/heap_file.h"
#include "semantic/semantic.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

// Table name
// Mapping between table names and their Heapfiles

class TableManager {
    private:
        std::string dataDir_;
        Catalog catalog_;
        // tableName -> HeapFile
        std::unordered_map<std::string, std::unique_ptr<HeapFile>> heapFiles_;

        // Path to .db file to a table
        std::string tablePath(const std::string& tableName) const;
    public:
        explicit TableManager(const std::string& dataDir);
        // Create new table
        void createTable(const std::string& tableName, const std::vector<ColumnDefinition>& columns);
        // Returns true if table exist
        bool hasTable(const std::string& tableName) const;
        // Get HeapFile for a table
        HeapFile& getHeapFile(const std::string& tableName);
        // Get schema for a table
        const TableSchema& getTableSchema(const std::string& tableName) const;

        // Get catalog
        Catalog& getCatalog() {
            return catalog_;
        }
};

#endif //TABLE_MANAGER_H