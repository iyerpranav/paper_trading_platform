#include "Stock.h"

Stock::Stock(const std::string& symbol, int quantity, double purchasePrice)
    : symbol(symbol), quantity(quantity), purchasePrice(purchasePrice) {}

std::string Stock::getSymbol() const {
    return symbol;
}

int Stock::getQuantity() const {
    return quantity;
}

double Stock::getPurchasePrice() const {
    return purchasePrice;
}

void Stock::addShares(int amount, double price) {
    double totalCost = (purchasePrice * quantity) + (price * amount);
    quantity += amount;
    if (quantity > 0) {
        purchasePrice = totalCost / quantity;
    }
}

void Stock::removeShares(int amount) {
    quantity -= amount;
}
