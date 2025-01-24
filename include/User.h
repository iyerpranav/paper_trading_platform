#ifndef USER_H
#define USER_H

#include "Stock.h"
#include "Portfolio.h"
#include <iostream>
#include <fstream>
#include <string>

class User : protected Stock, protected Portfolio {
private:
    std::string username, password;
    bool validateUser();

public:
    void authenticate();
    void viewStockData();
    void viewStockTrend();
    void viewPortfolio();
    void buyStock();
    void sellStock();
};

#endif // USER_H
