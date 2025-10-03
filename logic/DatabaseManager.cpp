#include "DatabaseManager.h"
#include "sqlite3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include "Stock.h"
#include "json.hpp"
using ordered_json = nlohmann::ordered_json;


DatabaseManager::DatabaseManager(const std::string &db_path) : db_file(db_path) {}
DatabaseManager::~DatabaseManager() {}

bool DatabaseManager::initializeDatabase()
{
    sqlite3 *db;
    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "[ERROR] Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char *sql_schema = R"SQL(
        -- User table: stores user information
        CREATE TABLE IF NOT EXISTS User (
            UserID INTEGER PRIMARY KEY AUTOINCREMENT,
            Username TEXT NOT NULL UNIQUE,
            Email TEXT NOT NULL UNIQUE,
            PasswordHash TEXT NOT NULL,
            CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );

        -- Stock table: stores stock information
        CREATE TABLE IF NOT EXISTS Stock (
            StockID INTEGER PRIMARY KEY AUTOINCREMENT,
            Symbol TEXT NOT NULL UNIQUE,
            CompanyName TEXT NOT NULL,
            MarketCap INTEGER,
            AvgVolume INTEGER,
            DividendYield REAL,
            PERatio REAL,
            FiftyTwoWeekLow REAL,
            FiftyTwoWeekHigh REAL,
            DayLow REAL,
            DayHigh REAL,
            PreviousClose REAL
        );

        -- Portfolio table: stores user's portfolio summary
        CREATE TABLE IF NOT EXISTS Portfolio (
            PortfolioID INTEGER PRIMARY KEY AUTOINCREMENT,
            UserID INTEGER NOT NULL,
            TotalValue REAL DEFAULT 0,
            CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (UserID) REFERENCES User(UserID)
        );

        -- Transaction table: records trades made by User
        CREATE TABLE IF NOT EXISTS UserTransaction (
            TransactionID INTEGER PRIMARY KEY AUTOINCREMENT,
            UserID INTEGER NOT NULL,
            StockID INTEGER NOT NULL,
            TransactionType TEXT NOT NULL, -- e.g., 'buy' or 'sell'
            Quantity INTEGER NOT NULL,
            Price REAL NOT NULL,
            Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (UserID) REFERENCES User(UserID),
            FOREIGN KEY (StockID) REFERENCES Stock(StockID)
        );

        -- MarketData table: stores market-related data for stocks
        CREATE TABLE IF NOT EXISTS MarketData (
            MarketDataID INTEGER PRIMARY KEY AUTOINCREMENT,
            StockID INTEGER NOT NULL,
            Price REAL NOT NULL,
            Volume INTEGER,
            Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (StockID) REFERENCES Stock(StockID)
        );

        -- Order table: stores user orders (pending/completed/cancelled)
        CREATE TABLE IF NOT EXISTS OrderTable (
            OrderID INTEGER PRIMARY KEY AUTOINCREMENT,
            UserID INTEGER NOT NULL,
            StockID INTEGER NOT NULL,
            OrderType TEXT NOT NULL, -- e.g., market, limit
            Quantity INTEGER NOT NULL,
            Price REAL, -- price per share for limit orders
            Status TEXT NOT NULL, -- e.g., pending, completed, canceled
            Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (UserID) REFERENCES User(UserID),
            FOREIGN KEY (StockID) REFERENCES Stock(StockID)
        );
    )SQL";
        
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql_schema, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "[ERROR] SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_exec(db, "INSERT OR IGNORE INTO User (UserID, Username, PasswordHash) VALUES (1, 'test', '123');", 0, 0, 0);

    sqlite3_close(db);
    return true;
}

bool DatabaseManager::addUser(const std::string& username, const std::string& passwordHash, const std::string& email, int& user_id) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool success = false;

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) return false;

    const char* sql = "INSERT INTO User (Username, Email, PasswordHash) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {   
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, passwordHash.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            success = true;
            user_id = static_cast<int>(sqlite3_last_insert_rowid(db));
        }
    }
        
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

bool DatabaseManager::validateUser(const std::string &username, const std::string &password, int &user_id)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    bool success = false;

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK)
        return false;

    const char *sql = "SELECT UserID FROM User WHERE Username = ? AND PasswordHash = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            user_id = sqlite3_column_int(stmt, 0);
            success = true;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

json DatabaseManager::getAllStocksAsJson()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    ordered_json stocks_array = json::array();

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK)
    {
        throw std::runtime_error("Cannot open database");
    }

    const char *sql = R"SQL(
        SELECT 
            s.Symbol,
            s.CompanyName,
            md.Price, 
            md.Volume,
            s.DayHigh,
            s.DayLow,
            s.PreviousClose,
            s.PERatio,
            s.DividendYield,
            s.FiftyTwoWeekLow,
            s.FiftyTwoWeekHigh
        FROM 
            MarketData md
        JOIN
            Stock s ON md.StockID = s.StockID;
    )SQL";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            ordered_json stock_obj;
            stock_obj["Symbol"] = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            stock_obj["CompanyName"] = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            stock_obj["Price"] = sqlite3_column_double(stmt, 2);
            stock_obj["Volume"] = sqlite3_column_int(stmt, 3);
            stock_obj["DayHigh"] = sqlite3_column_double(stmt, 4);
            stock_obj["DayLow"] = sqlite3_column_double(stmt, 5);
            stock_obj["PreviousClose"] = sqlite3_column_double(stmt, 6);
            stock_obj["PERatio"] = sqlite3_column_double(stmt, 7);
            stock_obj["DividendYield"] = sqlite3_column_double(stmt, 8);
            stock_obj["FiftyTwoWeekLow"] = sqlite3_column_double(stmt, 9);
            stock_obj["FiftyTwoWeekHigh"] = sqlite3_column_double(stmt, 10);
            stocks_array.push_back(stock_obj);
        }
    }
    else
    {
        std::cerr << "[ERROR] Failed to prepare statement for getting all stocks: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return stocks_array;
}

bool DatabaseManager::loadPortfolio(int user_id, Portfolio &portfolio)
{
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool success = false;

    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    const char* sql = R"SQL(
        SELECT
            s.Symbol,
            SUM(CASE WHEN t.TransactionType = 'buy' THEN t.Quantity ELSE 0 END) -
            SUM(CASE WHEN t.TransactionType = 'sell' THEN t.Quantity ELSE 0 END) AS QuantityHeld,
            md.Price AS LatestPrice
        FROM
            UserTransaction t
        JOIN
            Stock s ON t.StockID = s.StockID
        LEFT JOIN
            MarketData md ON s.StockID = md.StockID
        WHERE
            t.UserID = ?
            AND md.Timestamp = (
                SELECT MAX(Timestamp)
                FROM MarketData
                WHERE StockID = s.StockID
            )
        GROUP BY
            s.StockID
        HAVING
            QuantityHeld > 0;
    )SQL";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int quantity = sqlite3_column_int(stmt, 1);
            double latestPrice = sqlite3_column_double(stmt, 2);

            // Create Stock object using symbol, quantity, latest price
            Stock s(symbol, quantity, latestPrice);
            portfolio.addStock(s);

            success = true;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return success;
}


bool DatabaseManager::savePortfolio(int user_id, const Portfolio &portfolio)
{
    // Logic to save funds and stocks from the portfolio object for the user_id
    return true; // placeholder
}

bool DatabaseManager::updateStockDatabase(const std::string &csv_path)
{
    // Logic to read CSV and update the stocks table
    return true; // placeholder
}
