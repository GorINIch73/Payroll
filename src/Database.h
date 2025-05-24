#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>

class Database {
public:
    Database(const std::string& dbPath);
    ~Database();

    bool open();
    void close();
    bool execute(const std::string& sql);
    bool createNewDatabase();
    bool tableExists(const std::string& tableName);
    
    sqlite3* getHandle() const { return db; }

private:
    sqlite3* db;
    std::string dbPath;
};

