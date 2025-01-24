#ifndef STOCK_H
#define STOCK_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>

class Stock {
public:
    void updateStockInfo();
    void displayInfo(int option = 0);
    bool validateStockName(const std::string& stockName, const std::string& filename);
};

#endif // STOCK_H
