// PdfExporter.h
#pragma once
#include "Database.h"
#include <map>
#include <string>
#include <utility>
#include <vector>
// Структура для хранения данных из БД
struct TableRow {
        std::vector<std::string> columns;
};

class PdfExporter {
    public:
        PdfExporter(Database &db, std::string qr);
        void getData();
        void OpenPDF(const char *filename);
        void GeneratePDFListEmployees(const char *filename);
        bool
        ExportToPdf(const std::vector<std::map<std::string, std::string>> &data,
                    const std::string &filename);

    private:
        Database &db;
        std::vector<TableRow> data;
        std::string query;
};
