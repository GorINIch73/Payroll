// Settings.h  
#pragma once  
#include "Database.h"
// #include "Sett /ingsPanel.h"
#include <string>  
#include <nlohmann/json.hpp>  

class Settings {  
public:  
    void Load(const std::string& path);  
    void Save(const std::string& path);
    void getDataFromDB(Database &db);

    std::string lastDbPath;  
    int theme = 0; 
    std::string organization="";
      
};  
