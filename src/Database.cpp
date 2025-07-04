#include "Database.h"
#include <cstdio>
#include <iostream>

Database::Database()
    : db(nullptr) {

    // хз что ту делать
}

bool Database::open(const std::string &Path) {
    // если была открыта - закрыть
    close();

    dbPath = Path;
    // открываем новую
    int rc = sqlite3_open(dbPath.c_str(), &db);
    // Проверка на ошибки
    if (rc != SQLITE_OK || db == nullptr) {
        std::cerr << "Ошибка открытия базы: "
                  << (db ? sqlite3_errmsg(db) : "неизвестная ошибка")
                  << std::endl;

        if (db)
            sqlite3_close(db);
        return false;
    }

    return true;
}

Database::~Database() { close(); }

void Database::close() {

    if (db) {
        sqlite3_close(db); // Закрываем соединение с БД
        db = nullptr;      // Обнуляем указатель
    }
}

bool Database::execute(const std::string &sql) {

    if (!db)
        return false;

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::createNewDatabase() {

    if (!db)
        return false;

    const char *sql = R"(

    CREATE TABLE IF NOT EXISTS Individuals (
        id        INTEGER PRIMARY KEY AUTOINCREMENT
                          NOT NULL,
        full_name TEXT,
        note      TEXT
    );

    CREATE TABLE IF NOT EXISTS Orders (
        id             INTEGER PRIMARY KEY AUTOINCREMENT
                               NOT NULL,
        number         TEXT,
        date           TEXT    DEFAULT ('2000-01-01'),
        found          INTEGER DEFAULT (0),
        protocol_found INTEGER DEFAULT (0) 
    );


    CREATE TABLE IF NOT EXISTS Positions (
        id        INTEGER PRIMARY KEY AUTOINCREMENT
                          NOT NULL,
        job_title TEXT,
        salary    REAL    DEFAULT (0.0),
        norm      REAL    DEFAULT (0.0),
        note      TEXT
    );

    CREATE TABLE IF NOT EXISTS Accruals (
        id         INTEGER PRIMARY KEY AUTOINCREMENT
                           NOT NULL,
        name       TEXT,
        percentage REAL
    );

    CREATE TABLE IF NOT EXISTS Employees (
        id                INTEGER PRIMARY KEY AUTOINCREMENT
                                  NOT NULL,
        individual_id     INTEGER REFERENCES Individuals (id) ON DELETE NO ACTION
                                                              ON UPDATE NO ACTION,
        position_id       INTEGER REFERENCES Positions (id) ON DELETE NO ACTION
                                                            ON UPDATE NO ACTION,
        rate              REAL    DEFAULT (1),
        contract          TEXT,
        contract_found    INTEGER DEFAULT (0),
        certificate_found INTEGER DEFAULT (0),
        note              TEXT
    );


    CREATE TABLE IF NOT EXISTS Statements (
        id                 INTEGER PRIMARY KEY AUTOINCREMENT
                                   NOT NULL,
        month              INTEGER DEFAULT (1),
        employee_id        INTEGER REFERENCES Employees (id) 
                                   NOT NULL,
        hours_worked       REAL    DEFAULT (0),
        timesheet_verified INTEGER DEFAULT (0) 
    );


    CREATE TABLE IF NOT EXISTS List_accruals (
        id           INTEGER PRIMARY KEY AUTOINCREMENT
                             NOT NULL,
        statement_id INTEGER NOT NULL
                             REFERENCES Statements (id),
        accrual_id   INTEGER NOT NULL
                             REFERENCES Accruals (id),
        amount       REAL    DEFAULT (0.0),
        order_id     INTEGER REFERENCES Orders (id),
        verified     INTEGER DEFAULT (0),
        note         TEXT
    );

    )";

    return execute(sql);
}

bool Database::tableExists(const std::string &tableName) {
    std::string sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND name='" +
        tableName + "';";
    bool exists = false;
    sqlite3_exec(
        db, sql.c_str(),
        [](void *data, int, char **, char **) -> int {
            *static_cast<bool *>(data) = true;
            return 0;
        },
        &exists, nullptr);
    return exists;
}
