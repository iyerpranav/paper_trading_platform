#include "Portfolio.h"
#include <iostream>
#include <algorithm>

// Default constructor implementation
Portfolio::Portfolio() : fundBalance(10000.0) {} // Default starting balance

// New constructor implementation
Portfolio::Portfolio(double initial_balance) : fundBalance(initial_balance) {}

double Portfolio::getFundBalance() const {
    return fundBalance;
}

const std::vector<std::shared_ptr<Stock>>& Portfolio::getStocks() const {
    return stocks;
}

void Portfolio::setFundBalance(double balance) {
    fundBalance = balance;
}

void Portfolio::addStock(const Stock& stock) {
    stocks.push_back(std::make_shared<Stock>(stock));
}

bool Portfolio::buyStock(const std::string& symbol, int quantity, double price) {
    double totalCost = quantity * price;
    if (fundBalance < totalCost) {
        std::cerr << "[ERROR] Insufficient funds." << std::endl;
        return false;
    }

    // Find if stock already exists
    auto it = std::find_if(stocks.begin(), stocks.end(), 
        [&](const auto& s) { return s->getSymbol() == symbol; });
        
    if (it != stocks.end()) { // Stock exists
        (*it)->addToQuantity(quantity);
    } else { // New stock
        stocks.push_back(std::make_shared<Stock>(symbol, quantity, price));
    }
    
    fundBalance -= totalCost;
    return true;
}

bool Portfolio::sellStock(const std::string& symbol, int quantity, double price) {
    auto it = std::find_if(stocks.begin(), stocks.end(), 
        [&](const auto& s) { return s->getSymbol() == symbol; });

    if (it == stocks.end() || (*it)->getQuantity() < quantity) {
        std::cerr << "[ERROR] Not enough shares to sell." << std::endl;
        return false;
    }
    
    (*it)->removeFromQuantity(quantity);
    
    // If quantity is zero, remove the stock from portfolio
    if ((*it)->getQuantity() == 0) {
        stocks.erase(it);
    }

    fundBalance += quantity * price;
    return true;
}

