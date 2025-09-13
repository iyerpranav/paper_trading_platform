#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include "Stock.h"
#include <vector>
#include <string>
#include <memory>

/**
 * @class Portfolio
 * @brief Manages a collection of stocks and the user's fund balance.
 *
 * This class handles the business logic for buying and selling stocks,
 * updating the portfolio, and managing funds.
 */
class Portfolio {
public:
    /**
     * @brief Constructs a new Portfolio object.
     * @param initialBalance The starting fund balance for the portfolio.
     */
    Portfolio(double initialBalance);

    /**
     * @brief Buys a specified quantity of a stock at a given price.
     * @param symbol The stock to buy.
     * @param quantity The number of shares to buy.
     * @param price The price per share.
     * @return True if the transaction is successful, false otherwise.
     */
    bool buyStock(const std::string& symbol, int quantity, double price);

    /**
     * @brief Sells a specified quantity of a stock at a given price.
     * @param symbol The stock to sell.
     * @param quantity The number of shares to sell.
     * @param price The price per share.
     * @return True if the transaction is successful, false otherwise.
     */
    bool sellStock(const std::string& symbol, int quantity, double price);

    /**
     * @brief Gets the current fund balance.
     * @return The available fund balance.
     */
    double getFundBalance() const;

    /**
     * @brief Provides access to the stocks in the portfolio.
     * @return A constant reference to the vector of stocks.
     */
    const std::vector<std::unique_ptr<Stock>>& getStocks() const;
    
    /**
     * @brief Loads the portfolio state from a vector of string vectors.
     * This is typically used with the DatabaseManager.
     * @param data The portfolio data to load.
     */
    void loadFromData(const std::vector<std::vector<std::string>>& data);

private:
    double fundBalance;
    std::vector<std::unique_ptr<Stock>> stocks;

    /**
     * @brief Finds a stock in the portfolio by its symbol.
     * @param symbol The symbol of the stock to find.
     * @return A pointer to the Stock object, or nullptr if not found.
     */
    Stock* findStock(const std::string& symbol);
};

#endif // PORTFOLIO_H
