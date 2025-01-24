#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <thread>

class Portfolio {
protected:
    double fundBalance = 10000.00;
    double stockPrice, buyPrice, sellPrice;
    double totalCost, totalValue, profLoss;
    int holdQty;
    std::string stockName;
    int quantity;

public:
    void updatePortfolio();
    void displayPortfolio();
    void getBuySellInfo(const std::string& name, int option = 0);
    void executeBuyTransaction();
    void executeSellTransaction();
};

#endif // PORTFOLIO_H
