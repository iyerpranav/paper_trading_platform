#include "User.h"
#include "DatabaseManager.h"
#include <iostream>
#include <limits>
#include <iomanip>
#include <cstdlib>

// Forward declarations for functions in main
void displayMainMenu(User& user, DatabaseManager& dbManager);
void handleBuyStock(User& user, DatabaseManager& dbManager);
void handleSellStock(User& user, DatabaseManager& dbManager);
void displayPortfolio(User& user);
void displayStockInfo(DatabaseManager& dbManager);
void updateStockInfo(DatabaseManager& dbManager);
std::unique_ptr<User> login(DatabaseManager& dbManager);

const char* DB_FILE = "stock_portfolio.db";

/**
 * @brief Main entry point for the application.
 */
int main() {
    DatabaseManager dbManager(DB_FILE);
    if (!dbManager.initializeDatabase()) {
        std::cerr << "[FATAL] Could not initialize database. Exiting." << std::endl;
        return 1;
    }
    
    auto user = login(dbManager);

    if (user) {
        dbManager.loadPortfolio(user->getUserId(), user->getPortfolio());
        displayMainMenu(*user, dbManager);
    }

    return 0;
}

/**
 * @brief Handles the user login process.
 * @param dbManager The database manager to use for user validation.
 * @return A unique pointer to the logged-in User object, or nullptr if login fails.
 */
std::unique_ptr<User> login(DatabaseManager& dbManager) {
    std::string username, password;
    int attempts = 0;
    const int maxAttempts = 3;
    int userId = -1;

    std::cout << "\n================ LOGIN ====================\n";
    while (attempts < maxAttempts) {
        std::cout << "Enter Username: ";
        std::cin >> username;
        std::cout << "Enter Password: ";
        std::cin >> password;

        if (dbManager.validateUser(username, password, userId)) {
            std::cout << "\n[INFO] Welcome, " << username << "!" << std::endl;
            return std::make_unique<User>(username, userId);
        } else {
            std::cerr << "\n[ERROR] Invalid credentials." << std::endl;
            attempts++;
        }
    }
    std::cerr << "\n[ERROR] Maximum login attempts exceeded." << std::endl;
    return nullptr;
}

/**
 * @brief Displays the main menu and handles user choices.
 * @param user The current user.
 * @param dbManager The database manager.
 */
void displayMainMenu(User& user, DatabaseManager& dbManager) {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n================ MAIN MENU =================\n";
        std::cout << "1. Update Stock Info from CSV\n";
        std::cout << "2. Display Stock Info\n";
        std::cout << "3. Display Portfolio\n";
        std::cout << "4. Buy Stock\n";
        std::cout << "5. Sell Stock\n";
        std::cout << "0. Save & Exit\n";
        std::cout << "Enter your choice: ";
        
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1; // Invalid input
        }

        switch (choice) {
            case 1: updateStockInfo(dbManager); break;
            case 2: displayStockInfo(dbManager); break;
            case 3: displayPortfolio(user); break;
            case 4: handleBuyStock(user, dbManager); break;
            case 5: handleSellStock(user, dbManager); break;
            case 0: 
                if(dbManager.savePortfolio(user.getUserId(), user.getPortfolio())) {
                    std::cout << "\n[INFO] Portfolio saved. Exiting program. Goodbye!\n"; 
                } else {
                    std::cout << "\n[ERROR] Failed to save portfolio.\n";
                }
                break;
            default: 
                std::cerr << "\n[ERROR] Invalid choice. Try again.\n"; 
                break;
        }
    }
}

/**
 * @brief Handles the process of buying a stock.
 * @param user The current user.
 * @param dbManager The database manager.
 */
void handleBuyStock(User& user, DatabaseManager& dbManager) {
    std::string stockName;
    int quantity;
    double buyPrice;

    std::cout << "\n[INFO] Your Current Fund Balance: $" << std::fixed << std::setprecision(2) << user.getPortfolio().getFundBalance() << std::endl; 
    std::cout << "Enter the name of the stock you want to buy: "; 
    std::cin >> stockName;

    if (!dbManager.validateStockName(stockName)) {
        std::cerr << "\n[ERROR] Stock not found in database. Please update stock info first." << std::endl;
        return;
    }

    std::cout << "Enter the quantity of shares you want to buy: "; 
    if (!(std::cin >> quantity) || quantity <= 0) {
        std::cerr << "\n[ERROR] Invalid quantity. Enter a positive number." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    std::cout << "Enter the buy price: $"; 
    if (!(std::cin >> buyPrice) || buyPrice <= 0) {
        std::cerr << "\n[ERROR] Invalid price. Enter a positive number." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    if(user.getPortfolio().buyStock(stockName, quantity, buyPrice)) {
        std::cout << "\n[SUCCESS] Bought " << quantity << " shares of " << stockName << " at $" << buyPrice << " each.\n";
    }
}

/**
 * @brief Handles the process of selling a stock.
 * @param user The current user.
 * @param dbManager The database manager.
 */
void handleSellStock(User& user, DatabaseManager& dbManager) {
    std::string stockName;
    int quantity;
    double sellPrice;

    std::cout << "\n[INFO] Your Current Fund Balance: $" << user.getPortfolio().getFundBalance() << std::endl; 
    std::cout << "Enter the name of the stock you want to sell: "; 
    std::cin >> stockName;

    std::cout << "Enter the quantity of shares you want to sell: "; 
    if (!(std::cin >> quantity) || quantity <= 0) {
        std::cerr << "\n[ERROR] Invalid quantity. Enter a positive number." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    std::cout << "Enter the sell price: $"; 
    if (!(std::cin >> sellPrice) || sellPrice <= 0) {
        std::cerr << "\n[ERROR] Invalid price. Enter a positive number." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    if(user.getPortfolio().sellStock(stockName, quantity, sellPrice)) {
        std::cout << "\n[SUCCESS] Sold " << quantity << " shares of " << stockName << " at $" << sellPrice << " each.\n";
    }
}

/**
 * @brief Displays the user's current portfolio.
 * @param user The current user.
 */
void displayPortfolio(User& user) {
    const auto& stocks = user.getPortfolio().getStocks();
    if (stocks.empty()) {
        std::cout << "\n[INFO] Your portfolio is empty.\n";
        return;
    }

    std::cout << "\n======================= PORTFOLIO =======================\n";
    std::cout << std::left << std::setw(15) << "Stock" << std::setw(10) << "Qty" << std::setw(15) << "Avg Buy Price\n";
    std::cout << std::string(45, '-') << "\n";

    for (const auto& stock : stocks) {
        std::cout << std::left << std::setw(15) << stock->getSymbol() 
                  << std::setw(10) << stock->getQuantity() 
                  << std::setw(15) << std::fixed << std::setprecision(2) << stock->getPurchasePrice() << std::endl;
    }
    std::cout << std::string(45, '=') << "\n";
}

/**
 * @brief Displays general stock information from the database.
 * @param dbManager The database manager.
 */
void displayStockInfo(DatabaseManager& dbManager) {
    dbManager.displayAllStockInfo();
}

/**
 * @brief Updates the stock database from the CSV file.
 * @param dbManager The database manager.
 */
void updateStockInfo(DatabaseManager& dbManager) {
    // We keep the python script execution as it is what generates the csv
    int returnCode = system("python stockdb.py");
    if (returnCode == 0) {
        std::cout << "\n[INFO] Python stock program executed successfully." << std::endl;
        std::cout << "[INFO] Updating database from stock_info.csv..." << std::endl;
        if(dbManager.updateStockDatabase("stock_info.csv")) {
            std::cout << "[INFO] Database updated successfully." << std::endl;
        } else {
            std::cerr << "[ERROR] Failed to update database from CSV." << std::endl;
        }
    } else {
        std::cerr << "\n[ERROR] Failed to execute Python stock program." << std::endl;
    }
}

