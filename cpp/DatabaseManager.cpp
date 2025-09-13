#include "DatabaseManager.h"
#include "Portfolio.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

DatabaseManager::DatabaseManager(const char* dbFilename) : db(nullptr) {
    if (sqlite3_open(dbFilename, &db)) {
        std::cerr << "[ERROR] Can't open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::executeSql(const char* sql) {
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[ERROR] SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::initializeDatabase() {
    if (!db) return false;
    const char* createUserTable =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "password TEXT NOT NULL);";

    const char* createPortfolioTable =
        "CREATE TABLE IF NOT EXISTS portfolio ("
        "user_id INTEGER NOT NULL, "
        "symbol TEXT NOT NULL, "
        "quantity INTEGER NOT NULL, "
        "purchase_price REAL NOT NULL, "
        "FOREIGN KEY(user_id) REFERENCES users(id));";

    const char* createStockInfoTable =
        "CREATE TABLE IF NOT EXISTS stocks ("
        "symbol TEXT PRIMARY KEY NOT NULL, "
        "prev_close TEXT, "
        "day_range TEXT, "
        "year_range TEXT, "
        "market_cap TEXT, "
        "avg_volume TEXT, "
        "div_yield TEXT, "
        "pe_ratio TEXT);";

    // Example of adding a default user
    const char* insertDefaultUser = 
        "INSERT OR IGNORE INTO users (username, password) VALUES ('test', '123');";

    return executeSql(createUserTable) &&
           executeSql(createPortfolioTable) &&
           executeSql(createStockInfoTable) &&
           executeSql(insertDefaultUser);
}

bool DatabaseManager::validateUser(const std::string& username, const std::string& password, int& userId) {
    if (!db) return false;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id FROM users WHERE username = ? AND password = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "[ERROR] Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userId = sqlite3_column_int(stmt, 0);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::loadPortfolio(int userId, Portfolio& portfolio) {
    if (!db) return false;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT symbol, quantity, purchase_price FROM portfolio WHERE user_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "[ERROR] Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, userId);

    std::vector<std::vector<std::string>> data;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> row;
        row.push_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        row.push_back(std::to_string(sqlite3_column_int(stmt, 1)));
        row.push_back(std::to_string(sqlite3_column_double(stmt, 2)));
        data.push_back(row);
    }
    
    portfolio.loadFromData(data);

    sqlite3_finalize(stmt);
    return true;
}

bool DatabaseManager::savePortfolio(int userId, const Portfolio& portfolio) {
    if (!db) return false;
    
    char* zErrMsg = nullptr;
    if(sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "[ERROR] Begin transaction failed: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    sqlite3_stmt* deleteStmt;
    const char* deleteSql = "DELETE FROM portfolio WHERE user_id = ?;";
    if (sqlite3_prepare_v2(db, deleteSql, -1, &deleteStmt, 0) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return false;
    }
    sqlite3_bind_int(deleteStmt, 1, userId);
    sqlite3_step(deleteStmt);
    sqlite3_finalize(deleteStmt);

    sqlite3_stmt* insertStmt;
    const char* insertSql = "INSERT INTO portfolio (user_id, symbol, quantity, purchase_price) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insertSql, -1, &insertStmt, 0) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return false;
    }

    for (const auto& stock : portfolio.getStocks()) {
        sqlite3_bind_int(insertStmt, 1, userId);
        sqlite3_bind_text(insertStmt, 2, stock->getSymbol().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(insertStmt, 3, stock->getQuantity());
        sqlite3_bind_double(insertStmt, 4, stock->getPurchasePrice());
        
        if (sqlite3_step(insertStmt) != SQLITE_DONE) {
             std::cerr << "[ERROR] Failed to insert row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(insertStmt);
    }
    sqlite3_finalize(insertStmt);
    
    if (sqlite3_exec(db, "COMMIT;", 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "[ERROR] Commit failed: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}


bool DatabaseManager::validateStockName(const std::string& stockSymbol) {
    if (!db) return false;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT 1 FROM stocks WHERE symbol = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return false;
    }
    
    std::string upperSymbol = stockSymbol;
    for(char &c : upperSymbol) c = toupper(c);

    sqlite3_bind_text(stmt, 1, upperSymbol.c_str(), -1, SQLITE_STATIC);

    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::updateStockDatabase(const std::string& csvFilename) {
    std::ifstream ifile(csvFilename);
    if (!ifile.is_open()) {
        std::cerr << "\n[ERROR] Could not open " << csvFilename << " for reading." << std::endl;
        return false;
    }

    executeSql("DELETE FROM stocks;");
    
    char* zErrMsg = nullptr;
    if(sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "[ERROR] Begin transaction failed: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO stocks (symbol, prev_close, day_range, year_range, market_cap, avg_volume, div_yield, pe_ratio) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return false;
    }

    std::string line;
    while(getline(ifile, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while(getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if(tokens.size() >= 8) {
             for(int i = 0; i < 8; ++i) {
                sqlite3_bind_text(stmt, i + 1, tokens[i].c_str(), -1, SQLITE_STATIC);
            }
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "[ERROR] Failed to insert stock row: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_reset(stmt);
        }
    }
    sqlite3_finalize(stmt);
    ifile.close();
    
    if (sqlite3_exec(db, "COMMIT;", 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "[ERROR] Commit failed: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

void DatabaseManager::displayAllStockInfo() {
    if (!db) return;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT symbol, prev_close, day_range FROM stocks;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "[ERROR] Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "\n======================= STOCK DATA (from DB) =======================\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Symbol: " << sqlite3_column_text(stmt, 0)
                  << ", Prev Close: " << sqlite3_column_text(stmt, 1)
                  << ", Day Range: " << sqlite3_column_text(stmt, 2) << std::endl;
    }
    std::cout << "====================================================================\n";

    sqlite3_finalize(stmt);
}

