#include "PdfExporter.h"
#include "Database.h"
#include <cstddef>
#include <hpdf.h>
#include <hpdf_font.h>
#include <iostream>
// #include "hpdf.h"

#include <codecvt>
#include <cstdlib> // для system()
#include <locale>

PdfExporter::PdfExporter(Database &db, std::string qr)
    : db(db),
      query(qr) {
    getData();
    // std::cout << "проехали конструктор " << std::endl;
}

void PdfExporter::getData() {

    // Основная таблица
    //

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db.getHandle(), query.c_str(), -1, &stmt, nullptr) !=
        SQLITE_OK) {
        return;
    }

    int columnCount = sqlite3_column_count(stmt);
    data.clear();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TableRow row;
        for (int i = 0; i < columnCount; ++i) {
            const char *text = (const char *)sqlite3_column_text(stmt, i);
            // std::cout << " add " << text << std::endl;
            row.columns.push_back(text ? text : "NULL");
        }
        data.push_back(row);
    }

    sqlite3_finalize(stmt);
    // std::cout << "refresh  SQL query" << std::endl;
}

// Генерация PDF с данными из SQL
void PdfExporter::GeneratePDFListEmployees(const char *filename) {
    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    // поддержка кирилицы
    HPDF_UseUTFEncodings(pdf); // Включить UTF-8
    HPDF_Font font = HPDF_GetFont(
        pdf, HPDF_LoadTTFontFromFile(pdf, "NotoSans-Regular.ttf", HPDF_TRUE),
        "UTF-8");

    if (!font) {
        std::cerr << "Ошибка: не удалось загрузить шрифт" << std::endl;
        HPDF_Free(pdf);
        return;
    }
    // Устанавливаем шрифт и размер
    HPDF_Page_SetFontAndSize(page, font, 14);

    // HPDF_Page_BeginText(page);
    // HPDF_Page_MoveTextPos(page, 50, HPDF_Page_GetHeight(page) - 50);
    // HPDF_Page_ShowText(page, "Отчет из базы данных");
    // HPDF_Page_EndText(page);

    // Заголо:wвок
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 50, 750, "Список сортудников");
    HPDF_Page_EndText(page);

    // Таблица с данными
    float y = 700;
    HPDF_Page_SetFontAndSize(page, font, 12);

    // Заголовки столбцов
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 50, y, "ID");
    HPDF_Page_TextOut(page, 100, y, "ФИО");
    HPDF_Page_TextOut(page, 200, y, "Должность");
    HPDF_Page_TextOut(page, 300, y, "Ставок");
    HPDF_Page_TextOut(page, 400, y, "Подразделение");
    // HPDF_Page_TextOut(page, 300, y, "Salary");
    HPDF_Page_EndText(page);

    y -= 30;

    // Данные
    for (const auto &row : data) {
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 50, y, row.columns[0].c_str());
        HPDF_Page_TextOut(page, 100, y, row.columns[1].c_str());
        HPDF_Page_TextOut(page, 200, y, row.columns[2].c_str());
        HPDF_Page_TextOut(page, 300, y, row.columns[3].c_str());
        HPDF_Page_TextOut(page, 400, y, row.columns[4].c_str());
        // HPDF_Page_TextOut(page, 300, y,
        //                   ("$" + std::to_string(emp.salary)).c_str());
        HPDF_Page_EndText(page);
        y -= 20;
    }

    HPDF_SaveToFile(pdf, filename);
    HPDF_Free(pdf);
}

// Открытие PDF в стандартном приложении
void PdfExporter::OpenPDF(const char *filename) {
#ifdef _WIN32
    system(("start \"\" \"" + std::string(filename) + "\"").c_str());
#elif __APPLE__
    system(("open \"" + std::string(filename) + "\"").c_str());
#else
    system(("xdg-open \"" + std::string(filename) + "\"").c_str());
#endif
}

// Пример в ImGUI
// void ShowReportGenerator() {
//     static std::string dbPath = "company.db"; // Путь к БД
//     static std::string sqlQuery = "SELECT id, name, salary FROM employees;";
//
//     ImGui::Begin("SQL Report Generator");
//
//     ImGui::InputText("Database Path", &dbPath);
//     ImGui::InputTextMultiline("SQL Query", &sqlQuery);
//
//     if (ImGui::Button("Generate Report")) {
//         auto employees = FetchDataFromSQL(dbPath, sqlQuery);
//         GeneratePDFFromSQLData(employees, "employee_report.pdf");
//         OpenPDF("employee_report.pdf");
//     }
//
//     ImGui::End();
// }
//
bool PdfExporter::ExportToPdf(
    const std::vector<std::map<std::string, std::string>> &data,
    const std::string &filename) {
    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);
    if (!pdf)
        return false;

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    // поддержка кирилицы
    HPDF_UseUTFEncodings(pdf); // Включить UTF-8
    HPDF_Font font = HPDF_GetFont(
        pdf, HPDF_LoadTTFontFromFile(pdf, "NotoSans-Regular.ttf", HPDF_TRUE),
        "UTF-8");

    if (!font) {
        std::cerr << "Ошибка: не удалось загрузить шрифт" << std::endl;
        HPDF_Free(pdf);
        return false;
    }
    // Устанавливаем шрифт и размер
    HPDF_Page_SetFontAndSize(page, font, 14);

    HPDF_Page_BeginText(page);
    HPDF_Page_MoveTextPos(page, 50, HPDF_Page_GetHeight(page) - 50);
    // HPDF_Page_SetFontAndSize(page, HPDF_GetFont(pdf, "Helvetica", nullptr),
    // 12);

    // std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    // std::string utf8Text = converter.to_bytes(L"Привет");

    // Заголовок
    // HPDF_Page_ShowText(page, utf8Text);
    HPDF_Page_ShowText(page, "Отчет из базы данных");
    HPDF_Page_EndText(page);

    // Вывод данных
    float y = HPDF_Page_GetHeight(page) - 80;
    for (const auto &row : data) {
        std::string line;
        for (const auto &[key, value] : row) {
            line += key + ": " + value + " | ";
        }
        HPDF_Page_BeginText(page);
        HPDF_Page_MoveTextPos(page, 50, y);
        HPDF_Page_ShowText(page, line.c_str());
        HPDF_Page_EndText(page);
        y -= 20;
    }

    HPDF_SaveToFile(pdf, filename.c_str());
    HPDF_Free(pdf);
    return true;
}
