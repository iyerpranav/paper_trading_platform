#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include "Portfolio.h"
#include "json.hpp" // Include the JSON header

// Use nlohmann::json for convenience
using json = nlohmann::json;

class DatabaseManager {
private:
    std::string db_file;

public:
    DatabaseManager(const std::string& db_path);
    ~DatabaseManager();

    bool initializeDatabase();
    bool validateUser(const std::string& username, const std::string& password, int& user_id);
    bool loadPortfolio(int user_id, Portfolio& portfolio);
    bool savePortfolio(int user_id, const Portfolio& portfolio);
    bool updateStockDatabase(const std::string& csv_path);
    
    // New function to get all stocks as a JSON object
    json getAllStocksAsJson();
};

#endif // DATABASEMANAGER_H

