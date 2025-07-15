// Settings.h  
#pragma once  
#include <string>  
#include <nlohmann/json.hpp>  

class Settings {  
public:  
    void Load(const std::string& path);  
    void Save(const std::string& path);  

    std::string lastDbPath;  
    int theme = 0;  
    // ... другие настройки  
};  