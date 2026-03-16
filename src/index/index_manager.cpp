#include "index/index_manager.h"
#include <stdexcept>

void IndexManager::createIndex(const std::string& tableName, const std::string& columnName,
    const std::string& indexName, int columnIndex) {

    std::string key = indexKey(tableName, columnName);

    if (indexes_.count(key)) {
        throw std::runtime_error("Index " + tableName + " already exists");
    }

    indexes_[key] = std::make_unique<BPlusTree>();
    indexInfos_[key] = {tableName, columnName, indexName, columnIndex};
}

bool IndexManager::hasIndex(const std::string& tableName, const std::string& columnName) const {
    return indexes_.count(indexKey(tableName, columnName)) > 0;
}

BPlusTree& IndexManager::getIndex(const std::string& tableName, const std::string& columnName) {
    auto it = indexes_.find(indexKey(tableName, columnName));
    if (it == indexes_.end()) {
        throw std::runtime_error("Index " + tableName + " already exists");
    }
    return *it->second;
}

const IndexInfo& IndexManager::getIndexInfo(const std::string& tableName, const std::string& columnName) const {
    auto it = indexInfos_.find(indexKey(tableName, columnName));
    if (it == indexInfos_.end()) {
        throw std::runtime_error("Index " + tableName + " already exists");
    }
    return it->second;
}

void IndexManager::insertKey(const std::string& tableName, const std::string& columnName, BTreeKey key,
    RecordId recordId) {
        getIndex(tableName, columnName).insert(key, recordId);
}

void IndexManager::removeKey(const std::string& tableName, const std::string& columnName,BTreeKey key) {
    getIndex(tableName, columnName).remove(key);
}