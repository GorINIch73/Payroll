#include "PdfExporter.h"
#include <cstddef>
#include <hpdf.h>
#include <hpdf_font.h>
#include <iostream>
// #include "hpdf.h"

#include <codecvt>
#include <locale>

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
