// PdfExporter.h  
#pragma once  
#include <vector>  
#include <map>  
#include <string>  

class PdfExporter {  
public:  
    bool ExportToPdf(const std::vector<std::map<std::string, std::string>>& data, const std::string& filename);  
};  