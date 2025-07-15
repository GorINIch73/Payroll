#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

class Database {
    public:
        Database();
        ~Database();

        bool Open(const std::string &Path);
        void Close();
        bool Execute(const std::string &sql);
        bool CreateNewDatabase();
        bool tableExists(const std::string &tableName);

        bool IsOpen() const { return db != nullptr; }  

        sqlite3 *getHandle() const { return db; }

        std::vector<std::map<std::string, std::string>> FetchAll(const std::string& query);  
        
    private:
        sqlite3 *db;
        std::string dbPath;
};
