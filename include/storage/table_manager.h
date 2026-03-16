#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "storage/heap_file.h"
#include "storage/catalog.h"
#include "semantic/semantic.h"
#include "index/index_manager.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <vector>

// Mapping between table names and their HeapFiles
class TableManager {
private:
    std::string       dataDir_;
    Catalog           catalog_;
    IndexManager      indexManager_;
    PersistentCatalog persistentCatalog_;

    // tableName → HeapFile
    std::unordered_map<std::string, std::unique_ptr<HeapFile>> heapFiles_;

    // Path to .db file for a table
    std::string tablePath(const std::string& tableName) const;

public:
    explicit TableManager(const std::string& dataDir);

    // Create new table and persist to catalog
    void createTable(const std::string& tableName,
                     const std::vector<ColumnDefinition>& columns);

    // Returns true if table exists
    bool hasTable(const std::string& tableName) const;

    // Get HeapFile for a table
    HeapFile& getHeapFile(const std::string& tableName);

    // Get schema for a table
    const TableSchema& getTableSchema(const std::string& tableName) const;

    // Get all schemas (for saving catalog)
    std::vector<TableSchema> getAllSchemas() const;

    // Get index manager
    IndexManager& getIndexManager() { return indexManager_; }

    // Get catalog
    Catalog& getCatalog() { return catalog_; }
};

#endif // TABLE_MANAGER_H