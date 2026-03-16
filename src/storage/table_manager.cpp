#include "storage/table_manager.h"
#include <stdexcept>
#include <iostream>

TableManager::TableManager(const std::string& dataDir)
    : dataDir_(dataDir),
      persistentCatalog_(dataDir)
{
    std::filesystem::create_directories(dataDir);

    std::cout << "DEBUG: catalog exists = " << persistentCatalog_.exists() << "\n";

    if (persistentCatalog_.exists()) {
        auto schemas = persistentCatalog_.load();
        std::cout << "DEBUG: loaded " << schemas.size() << " schemas\n";
        for (const auto& schema : schemas) {
            std::cout << "DEBUG: loading table " << schema.name << "\n";
            catalog_.addTable(schema);
            heapFiles_[schema.name] =
                std::make_unique<HeapFile>(tablePath(schema.name));
        }
        if (!schemas.empty()) {
            std::cout << "[Catalog] Loaded " << schemas.size()
                      << " table(s) from disk.\n";
        }
    }
}

std::string TableManager::tablePath(const std::string& tableName) const {
    return dataDir_ + "/" + tableName + ".db";
}

void TableManager::createTable(const std::string& tableName,
                                const std::vector<ColumnDefinition>& columns) {
    if (catalog_.hasTable(tableName)) {
        throw std::runtime_error("Table '" + tableName + "' already exists");
    }

    TableSchema schema;
    schema.name    = tableName;
    schema.columns = columns;
    catalog_.addTable(schema);

    heapFiles_[tableName] = std::make_unique<HeapFile>(tablePath(tableName));

    // Persist catalog
    auto schemas = getAllSchemas();
    std::cout << "DEBUG: saving catalog with " << schemas.size() << " tables\n";
    std::cout << "DEBUG: catalog path = " << dataDir_ + "/catalog.dat" << "\n";
    persistentCatalog_.save(schemas);
    std::cout << "DEBUG: catalog saved\n";
    std::cout << "DEBUG: catalog exists after save = "
              << persistentCatalog_.exists() << "\n";
}

bool TableManager::hasTable(const std::string& tableName) const {
    return catalog_.hasTable(tableName);
}

HeapFile& TableManager::getHeapFile(const std::string& tableName) {
    auto it = heapFiles_.find(tableName);
    if (it == heapFiles_.end()) {
        throw std::runtime_error("Table '" + tableName + "' not found");
    }
    return *it->second;
}

const TableSchema& TableManager::getTableSchema(const std::string& tableName) const {
    return catalog_.getTable(tableName);
}

std::vector<TableSchema> TableManager::getAllSchemas() const {
    std::vector<TableSchema> schemas;
    for (const auto& [name, hf] : heapFiles_) {
        schemas.push_back(catalog_.getTable(name));
    }
    return schemas;
}