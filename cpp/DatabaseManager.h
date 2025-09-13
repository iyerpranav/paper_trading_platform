#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "sqlite3.h"
#include <string>
#include <vector>

class Portfolio; // Forward declaration

/**
 * @class DatabaseManager
 * @brief Handles all SQLite database interactions for the application.
 *
 * This class is responsible for connecting to the SQLite database,
 * initializing tables, and managing all data persistence for users,
 * portfolios, and stock information.
 */
class DatabaseManager {
public:
    /**
     * @brief Constructs a new DatabaseManager and connects to the database.
     * @param dbFilename The path to the SQLite database file.
     */
    DatabaseManager(const char* dbFilename);

    /**
     * @brief Destroys the DatabaseManager and closes the database connection.
     */
    ~DatabaseManager();

    /**
     * @brief Initializes the database by creating necessary tables if they don't exist.
     * @return True on success, false on failure.
     */
    bool initializeDatabase();

    /**
     * @brief Loads a user's portfolio from the database.
     * @param userId The ID of the user whose portfolio to load.
     * @param portfolio The Portfolio object to populate.
     * @return True on success, false on failure.
     */
    bool loadPortfolio(int userId, Portfolio& portfolio);

    /**
     * @brief Saves a user's portfolio to the database.
     * @param userId The ID of the user whose portfolio to save.
     * @param portfolio The Portfolio object containing the data to save.
     * @return True on success, false on failure.
     */
    bool savePortfolio(int userId, const Portfolio& portfolio);

    /**
     * @brief Validates user credentials against the database.
     * @param username The username to validate.
     * @param password The password to validate.
     * @param userId Output parameter to store the user's ID upon successful login.
     * @return True if credentials are valid, false otherwise.
     */
    bool validateUser(const std::string& username, const std::string& password, int& userId);
    
    /**
     * @brief Validates if a stock symbol exists in the database.
     * @param stockSymbol The symbol of the stock to validate.
     * @return True if the stock exists, false otherwise.
     */
    bool validateStockName(const std::string& stockSymbol);

    /**
     * @brief Updates the stocks table in the database from a CSV file.
     * @param csvFilename The path to the CSV file with stock data.
     * @return True on success, false otherwise.
     */
    bool updateStockDatabase(const std::string& csvFilename);

    /**
     * @brief Displays all stock information from the database.
     */
    void displayAllStockInfo();

private:
    sqlite3* db;
    bool executeSql(const char* sql);
};

#endif // DATABASE_MANAGER_H

