#include "DatabaseManager.h"
#include "sqlite3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include "Stock.h"

DatabaseManager::DatabaseManager(const std::string& db_path) : db_file(db_path) {}
DatabaseManager::~DatabaseManager() {}

bool DatabaseManager::initializeDatabase() {
    sqlite3* db;
    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[ERROR] Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* sql_users = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "username TEXT UNIQUE NOT NULL,"
                            "password TEXT NOT NULL);";
    // ... (rest of the table creation SQL is the same) ...

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql_users, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "[ERROR] SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    // Add default user if not exists
    sqlite3_exec(db, "INSERT OR IGNORE INTO users (id, username, password) VALUES (1, 'test', '123');", 0, 0, 0);

    sqlite3_close(db);
    return true;
}

bool DatabaseManager::validateUser(const std::string& username, const std::string& password, int& user_id) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool success = false;

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) return false;

    const char* sql = "SELECT id FROM users WHERE username = ? AND password = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user_id = sqlite3_column_int(stmt, 0);
            success = true;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

json DatabaseManager::getAllStocksAsJson() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    json stocks_array = json::array();

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database");
    }

    const char* sql = "SELECT symbol, prev_close, day_range, year_range, market_cap, avg_volume, div_yield, pe_ratio FROM stocks;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            json stock_obj;
            stock_obj["symbol"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            stock_obj["prev_close"] = sqlite3_column_double(stmt, 1);
            stock_obj["day_range"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            stock_obj["year_range"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            stock_obj["market_cap"] = sqlite3_column_int64(stmt, 4);
            stock_obj["avg_volume"] = sqlite3_column_int(stmt, 5);
            stock_obj["div_yield"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            stock_obj["pe_ratio"] = sqlite3_column_double(stmt, 7);
            stocks_array.push_back(stock_obj);
        }
    } else {
        std::cerr << "[ERROR] Failed to prepare statement for getting all stocks: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return stocks_array;
}

// Implementations for loadPortfolio, savePortfolio, updateStockDatabase remain largely the same,
// but ensure they handle errors robustly.
bool DatabaseManager::loadPortfolio(int user_id, Portfolio& portfolio) {
    // Logic to load funds and stocks for the user_id into the portfolio object
    return true; // placeholder
}

bool DatabaseManager::savePortfolio(int user_id, const Portfolio& portfolio) {
    // Logic to save funds and stocks from the portfolio object for the user_id
    return true; // placeholder
}

bool DatabaseManager::updateStockDatabase(const std::string& csv_path) {
    // Logic to read CSV and update the stocks table
    return true; // placeholder
}

