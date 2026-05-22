#include "HtmlExporter.h"
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <utility>

namespace {
std::string escapeHtml(const std::string &value) {
    std::string result;
    result.reserve(value.size());

    for (char c : value) {
        switch (c) {
        case '&':
            result += "&amp;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '"':
            result += "&quot;";
            break;
        case '\'':
            result += "&#39;";
            break;
        default:
            result += c;
            break;
        }
    }

    return result;
}
} // namespace

HtmlExporter::HtmlExporter(Database &db, std::string query)
    : db(db),
      query(std::move(query)) {
    loadData();
}

void HtmlExporter::loadData() {
    headers.clear();
    data.clear();

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db.getHandle(), query.c_str(), -1, &stmt, nullptr) !=
        SQLITE_OK) {
        std::cerr << "Failed to prepare report query: "
                  << sqlite3_errmsg(db.getHandle()) << std::endl;
        return;
    }

    int columnCount = sqlite3_column_count(stmt);
    for (int i = 0; i < columnCount; ++i) {
        const char *name = sqlite3_column_name(stmt, i);
        headers.emplace_back(name ? name : "");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        HtmlTableRow row;
        for (int i = 0; i < columnCount; ++i) {
            const char *text = (const char *)sqlite3_column_text(stmt, i);
            row.columns.emplace_back(text ? text : "");
        }
        data.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
}

bool HtmlExporter::GenerateEmployeesReport(const std::string &filename) {
    std::ofstream out(filename);
    if (!out.good()) {
        std::cerr << "Failed to create HTML report: " << filename << std::endl;
        return false;
    }

    out << "<!doctype html>\n"
        << "<html lang=\"ru\">\n"
        << "<head>\n"
        << "  <meta charset=\"utf-8\">\n"
        << "  <title>Список сотрудников</title>\n"
        << "  <style>\n"
        << "    body{font-family:Arial,sans-serif;margin:24px;color:#1f2933;}\n"
        << "    h1{font-size:22px;margin:0 0 16px;}\n"
        << "    table{border-collapse:collapse;width:100%;font-size:14px;}\n"
        << "    th,td{border:1px solid #cbd2d9;padding:6px 8px;text-align:left;}\n"
        << "    th{background:#eef2f6;}\n"
        << "    tr:nth-child(even){background:#f8fafc;}\n"
        << "  </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "  <h1>Список сотрудников</h1>\n"
        << "  <table>\n"
        << "    <thead><tr>";

    for (const auto &header : headers) {
        out << "<th>" << escapeHtml(header) << "</th>";
    }
    out << "</tr></thead>\n"
        << "    <tbody>\n";

    for (const auto &row : data) {
        out << "      <tr>";
        for (const auto &column : row.columns) {
            out << "<td>" << escapeHtml(column) << "</td>";
        }
        out << "</tr>\n";
    }

    out << "    </tbody>\n"
        << "  </table>\n"
        << "</body>\n"
        << "</html>\n";

    return true;
}
