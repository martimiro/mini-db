#include "storage/catalog.h"
#include <fstream>
#include <stdexcept>

PersistentCatalog::PersistentCatalog(const std::string& dataDir)
    : catalogPath_(dataDir + "/catalog.dat")
{}

bool PersistentCatalog::exists() const {
    return std::filesystem::exists(catalogPath_);
}

void PersistentCatalog::save(const std::vector<TableSchema>& schemas) {
    std::ofstream f(catalogPath_);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot write catalog: " + catalogPath_);
    }

    f << schemas.size() << "\n";
    for (const auto& schema : schemas) {
        f << schema.name << " " << schema.columns.size() << "\n";
        for (const auto& col : schema.columns) {
            std::string typeName = (col.type == TokenType::TOKEN_KEYWORD_INT)
                                   ? "INT" : "TEXT";
            f << col.name << " " << typeName << "\n";
        }
    }
}

std::vector<TableSchema> PersistentCatalog::load() {
    std::vector<TableSchema> schemas;
    std::ifstream f(catalogPath_);
    if (!f.is_open()) return schemas;

    int numTables;
    f >> numTables;

    for (int i = 0; i < numTables; i++) {
        TableSchema schema;
        int numCols;
        f >> schema.name >> numCols;

        for (int j = 0; j < numCols; j++) {
            ColumnDefinition col;
            std::string typeName;
            f >> col.name >> typeName;
            col.type = (typeName == "INT")
                       ? TokenType::TOKEN_KEYWORD_INT
                       : TokenType::TOKEN_KEYWORD_TEXT;
            schema.columns.push_back(col);
        }
        schemas.push_back(schema);
    }

    return schemas;
}