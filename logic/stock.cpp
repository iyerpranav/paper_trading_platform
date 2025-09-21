#include "Stock.h"

Stock::Stock(const std::string& sym, int qty, double price)
    : symbol(sym), quantity(qty), purchase_price(price) {}

std::string Stock::getSymbol() const {
    return symbol;
}

int Stock::getQuantity() const {
    return quantity;
}

double Stock::getPurchasePrice() const {
    return purchase_price;
}

void Stock::addToQuantity(int amount) {
    if (amount > 0) {
        quantity += amount;
    }
}

void Stock::removeFromQuantity(int amount) {
    if (amount > 0 && quantity >= amount) {
        quantity -= amount;
    }
}

