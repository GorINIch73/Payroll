#pragma once
#include "Database.h"
#include <string>
#include <vector>

struct HtmlTableRow {
        std::vector<std::string> columns;
};

class HtmlExporter {
    public:
        HtmlExporter(Database &db, std::string query);

        bool GenerateEmployeesReport(const std::string &filename);

    private:
        void loadData();

        Database &db;
        std::string query;
        std::vector<std::string> headers;
        std::vector<HtmlTableRow> data;
};
