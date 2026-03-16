#ifndef CATALOG_H
#define CATALOG_H

#include "semantic/semantic.h"
#include <string>
#include <vector>
#include <filesystem>

// Persistent catalog
// Saves and loads table schemas to/from disk as a simple text format.
class PersistentCatalog {
public:
    explicit PersistentCatalog(const std::string& dataDir);

    // Save all schemas to disk
    void save(const std::vector<TableSchema>& schemas);

    // Load all schemas from disk
    std::vector<TableSchema> load();

    // Check if catalog file exists
    bool exists() const;

private:
    std::string catalogPath_;
};

#endif //CATALOG_H