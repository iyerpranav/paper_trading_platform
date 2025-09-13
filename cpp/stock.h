#ifndef STOCK_H
#define STOCK_H

#include <string>

/**
 * @class Stock
 * @brief Represents a single stock with its essential data.
 *
 * This class holds information about a stock, such as its symbol,
 * quantity, and purchase price.
 */
class Stock {
public:
    /**
     * @brief Constructs a new Stock object.
     * @param symbol The stock's ticker symbol.
     * @param quantity The number of shares held.
     * @param purchasePrice The average price at which the shares were bought.
     */
    Stock(const std::string& symbol, int quantity, double purchasePrice);

    /**
     * @brief Gets the stock's symbol.
     * @return The stock symbol.
     */
    std::string getSymbol() const;

    /**
     * @brief Gets the quantity of shares.
     * @return The number of shares.
     */
    int getQuantity() const;

    /**
     * @brief Gets the average purchase price.
     * @return The purchase price per share.
     */
    double getPurchasePrice() const;

    /**
     * @brief Adds shares to the stock holding.
     * @param amount The number of shares to add.
     * @param price The price at which the new shares are bought.
     */
    void addShares(int amount, double price);

    /**
     * @brief Removes shares from the stock holding.
     * @param amount The number of shares to remove.
     */
    void removeShares(int amount);

private:
    std::string symbol;
    int quantity;
    double purchasePrice;
};

#endif // STOCK_H
