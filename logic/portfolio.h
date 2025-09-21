#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <vector>
#include <string>
#include <memory>
#include "Stock.h"

class Portfolio {
private:
    double fundBalance;
    std::vector<std::shared_ptr<Stock>> stocks;

public:
    Portfolio(); // Default constructor
    Portfolio(double initial_balance); // New constructor for initial funds
    
    // Public interface
    bool buyStock(const std::string& symbol, int quantity, double price);
    bool sellStock(const std::string& symbol, int quantity, double price);
    
    // Getters
    double getFundBalance() const;
    const std::vector<std::shared_ptr<Stock>>& getStocks() const;

    // Setters / Modifiers (used by DatabaseManager)
    void setFundBalance(double balance);
    void addStock(const Stock& stock);
};

#endif // PORTFOLIO_H

