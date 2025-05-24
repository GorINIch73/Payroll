#include "Database.h"
#include <iostream>

Database::Database(const std::string& dbPath) : dbPath(dbPath), db(nullptr) {}

bool Database::open() {
    return sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK;
}

Database::~Database()
{
  close();
}


void Database::close() {

    if (db) {
        sqlite3_close(db);  // Закрываем соединение с БД
        db = nullptr;       // Обнуляем указатель
    }

}

bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::createNewDatabase() {
    if (!open()) return false;

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS Employees (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            position TEXT,
            salary REAL
        );

        CREATE TABLE IF NOT EXISTS PayrollTypes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            coefficient REAL
        );
    )";

    return execute(sql);
}

bool Database::tableExists(const std::string& tableName) {
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
    bool exists = false;
    sqlite3_exec(db, sql.c_str(), [](void* data, int, char**, char**) -> int {
        *static_cast<bool*>(data) = true;
        return 0;
    }, &exists, nullptr);
    return exists;
}



