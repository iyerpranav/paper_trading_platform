#ifndef STOCK_H
#define STOCK_H

#include <string>

class Stock {
private:
    std::string symbol;
    int quantity;
    double purchase_price;

public:
    // Constructors
    Stock(const std::string& sym, int qty, double price);

    // Getters
    std::string getSymbol() const;
    int getQuantity() const;
    double getPurchasePrice() const;

    // Modifiers
    void addToQuantity(int amount);
    void removeFromQuantity(int amount);
};

#endif // STOCK_H

