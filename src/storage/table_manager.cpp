#include "storage/table_manager.h"
#include <stdexcept>

TableManager::TableManager(const std::string &dataDir) {
    this->dataDir_ = dataDir;
    std::filesystem::create_directories(dataDir);
}

std::string TableManager::tablePath(const std::string &tableName) const {
    return dataDir_ + "/" + tableName + ".db";
}

void TableManager::createTable(const std::string& tableName, const std::vector<ColumnDefinition>& columns) {
    if (catalog_.hasTable(tableName)) {
        throw std::logic_error("Table already exists");
    }

    // Register in catalog
    TableSchema schema;
    schema.name = tableName;
    schema.columns = columns;
    catalog_.addTable(schema);

    // Create Heapfile
    heapFiles_[tableName] = std::make_unique<HeapFile>(tablePath(tableName));
}

bool TableManager::hasTable(const std::string& tableName) const {
    return catalog_.hasTable(tableName);
}

HeapFile& TableManager::getHeapFile(const std::string& tableName) {
    auto itr = heapFiles_.find(tableName);
    if (itr == heapFiles_.end()) {
        throw std::logic_error("Table not found");
    }

    return *itr->second;
}

const TableSchema& TableManager::getTableSchema(const std::string& tableName) const {
    return catalog_.getTable(tableName);
}