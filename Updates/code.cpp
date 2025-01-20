#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <iomanip>
using namespace std;

class Stock {
private:
    unordered_map<string, pair<string, double>> stockDatabase; // Symbol -> {Name, Price}

public:
    void loadStockDatabase() {
        ifstream file("stocks.txt");
        if (!file.is_open()) {
            cout << "Error: Unable to load stock database.\n";
            return;
        }
        string symbol, name;
        double price;
        while (file >> symbol >> name >> price) {
            stockDatabase[symbol] = {name, price};
        }
        file.close();
    }

    bool validateStockSymbol(const string& symbol) {
        return stockDatabase.find(symbol) != stockDatabase.end();
    }

    pair<string, double> getStockData(const string& symbol) {
        return stockDatabase[symbol];
    }

    void displayStockData() {
        cout << left << setw(10) << "Symbol" << setw(20) << "Name" << "Price" << endl;
        for (const auto& stock : stockDatabase) {
            cout << setw(10) << stock.first << setw(20) << stock.second.first << stock.second.second << endl;
        }
    }
};

class Portfolio {
protected:
    double fundBalance;
    unordered_map<string, int> userPortfolio; // Symbol -> Quantity

public:
    Portfolio() : fundBalance(1000000.0) {}

    void loadPortfolioData(const string& username) {
        ifstream file(username + "_portfolio.txt");
        if (!file.is_open()) return;
        string symbol;
        int quantity;
        while (file >> symbol >> quantity) {
            userPortfolio[symbol] = quantity;
        }
        file.close();
    }

    void updatePortfolioData(const string& username) {
        ofstream file(username + "_portfolio.txt");
        for (const auto& entry : userPortfolio) {
            file << entry.first << " " << entry.second << endl;
        }
        file.close();
    }

    void executeBuyTransaction(const string& symbol, int quantity, double price) {
        double totalCost = quantity * price;
        if (totalCost > fundBalance) {
            cout << "Error: Insufficient funds.\n";
            return;
        }
        fundBalance -= totalCost;
        userPortfolio[symbol] += quantity;
        cout << "Purchase successful.\n";
    }

    void executeSellTransaction(const string& symbol, int quantity, double price) {
        if (userPortfolio[symbol] < quantity) {
            cout << "Error: Insufficient shares.\n";
            return;
        }
        userPortfolio[symbol] -= quantity;
        fundBalance += quantity * price;
        if (userPortfolio[symbol] == 0) {
            userPortfolio.erase(symbol);
        }
        cout << "Sale successful.\n";
    }

    void displayPortfolio() {
        cout << "Fund Balance: " << fundBalance << "\n";
        cout << left << setw(10) << "Symbol" << "Quantity" << endl;
        for (const auto& entry : userPortfolio) {
            cout << setw(10) << entry.first << entry.second << endl;
        }
    }
};

class User {
private:
    string currentUsername;
    string currentUserPassword;

    Stock stockManager;
    Portfolio portfolioManager;

public:
    bool authenticate(const string& username, const string& password) {
        ifstream file("users.txt");
        string user, pass;
        while (file >> user >> pass) {
            if (user == username && pass == password) {
                currentUsername = username;
                currentUserPassword = password;
                portfolioManager.loadPortfolioData(username);
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    void registerUser(const string& username, const string& password) {
        ofstream file("users.txt", ios::app);
        file << username << " " << password << endl;
        file.close();
        currentUsername = username;
        currentUserPassword = password;
        portfolioManager.loadPortfolioData(username);
    }

    void searchNSEStockData() {
        stockManager.displayStockData();
    }

    void buyStock() {
        string symbol;
        int quantity;
        cout << "Enter stock symbol to buy: ";
        cin >> symbol;
        if (!stockManager.validateStockSymbol(symbol)) {
            cout << "Error: Invalid stock symbol.\n";
            return;
        }
        auto stockData = stockManager.getStockData(symbol);
        cout << "Enter quantity to buy: ";
        cin >> quantity;
        portfolioManager.executeBuyTransaction(symbol, quantity, stockData.second);
    }

    void sellStock() {
        string symbol;
        int quantity;
        cout << "Enter stock symbol to sell: ";
        cin >> symbol;
        auto stockData = stockManager.getStockData(symbol);
        cout << "Enter quantity to sell: ";
        cin >> quantity;
        portfolioManager.executeSellTransaction(symbol, quantity, stockData.second);
    }

    void interact() {
        stockManager.loadStockDatabase();

        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        if (!authenticate(username, password)) {
            cout << "User not found. Registering new user.\n";
            registerUser(username, password);
        }

        int choice;
        do {
            cout << "\nMenu:\n";
            cout << "1. View Stock Data\n2. Buy Stock\n3. Sell Stock\n4. View Portfolio\n5. Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
            case 1:
                searchNSEStockData();
                break;
            case 2:
                buyStock();
                break;
            case 3:
                sellStock();
                break;
            case 4:
                portfolioManager.displayPortfolio();
                break;
            case 5:
                portfolioManager.updatePortfolioData(currentUsername);
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice.\n";
            }
        } while (choice != 5);
    }
};

int main() {
    User user;
    user.interact();
    return 0;
}
