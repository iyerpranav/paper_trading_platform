#include "Portfolio.h"
#include <iostream>
#include <stdexcept>

Portfolio::Portfolio(double initialBalance) : fundBalance(initialBalance) {}

bool Portfolio::buyStock(const std::string& symbol, int quantity, double price) {
    if (quantity <= 0 || price <= 0) {
        std::cerr << "\n[ERROR] Quantity and price must be positive." << std::endl;
        return false;
    }
    double totalCost = price * quantity;
    if (totalCost > fundBalance) {
        std::cerr << "\n[ERROR] Insufficient funds." << std::endl;
        return false;
    }

    fundBalance -= totalCost;
    Stock* stock = findStock(symbol);
    if (stock) {
        stock->addShares(quantity, price);
    } else {
        stocks.push_back(std::make_unique<Stock>(symbol, quantity, price));
    }
    return true;
}

bool Portfolio::sellStock(const std::string& symbol, int quantity, double price) {
    if (quantity <= 0 || price <= 0) {
        std::cerr << "\n[ERROR] Quantity and price must be positive." << std::endl;
        return false;
    }

    Stock* stock = findStock(symbol);
    if (!stock || stock->getQuantity() < quantity) {
        std::cerr << "\n[ERROR] You do not own enough shares of this stock." << std::endl;
        return false;
    }

    stock->removeShares(quantity);
    fundBalance += quantity * price;

    // Optional: remove stock from portfolio if quantity is zero
    if (stock->getQuantity() == 0) {
        for (auto it = stocks.begin(); it != stocks.end(); ++it) {
            if ((*it)->getSymbol() == symbol) {
                stocks.erase(it);
                break;
            }
        }
    }
    
    return true;
}

double Portfolio::getFundBalance() const {
    return fundBalance;
}

const std::vector<std::unique_ptr<Stock>>& Portfolio::getStocks() const {
    return stocks;
}

void Portfolio::loadFromData(const std::vector<std::vector<std::string>>& data) {
    stocks.clear();
    for(const auto& row : data) {
        if (row.size() >= 3) {
            try {
                std::string symbol = row[0];
                int quantity = std::stoi(row[1]);
                double price = std::stod(row[2]);
                stocks.push_back(std::make_unique<Stock>(symbol, quantity, price));
            } catch (const std::invalid_argument& e) {
                std::cerr << "[WARNING] Invalid data format in portfolio file for row, skipping." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "[WARNING] Numerical value out of range in portfolio file for row, skipping." << std::endl;
            }
        }
    }
}

Stock* Portfolio::findStock(const std::string& symbol) {
    for (const auto& stock : stocks) {
        if (stock->getSymbol() == symbol) {
            return stock.get();
        }
    }
    return nullptr;
}
