#include "D:/Projects/stock_portfolio/include/Stock.h"

void Stock::updateStockInfo() {
    std::thread t1([](){
        int returnCode = system("python scripts/stockdb.py");
        if (returnCode == 0) {
            std::cout << "\tPython stock program executed successfully." << std::endl;
        } else {
            std::cerr << "Error executing Python stock program." << std::endl;
        }
    });
    t1.join();
}

void Stock::displayInfo(int option) {
    std::ifstream ifile("data/stock_info.csv");
    if (ifile.fail()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    std::cout << "\n-----------------------STOCK DATA-----------------------\n\n";
    std::string line;
    char delimiter = ',';
    while (getline(ifile, line)) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 8) {
            if (option == 0) {
                std::cout << std::left << std::setw(15) << tokens[0];  
                std::cout << std::left << std::setw(20) << tokens[1];  
                std::cout << std::left << std::setw(30) << tokens[2];  
                std::cout << std::endl;
            } else {
                std::cout << std::left << std::setw(30) << tokens[3];  
                std::cout << std::left << std::setw(20) << tokens[4];  
                std::cout << std::left << std::setw(15) << tokens[5];  
                std::cout << std::left << std::setw(10) << tokens[6];  
                std::cout << std::left << std::setw(10) << tokens[7];  
                std::cout << std::endl;
            }
        } else {
            std::cout << "Invalid line format: " << line << std::endl;
        }
    }
    ifile.close();
}

bool Stock::validateStockName(const std::string& stockName, const std::string& filename) {
    std::ifstream ifile(filename);
    if (ifile.fail()) {
        std::cerr << "Error opening file." << std::endl;
        return false;
    }

    std::string line;
    while (getline(ifile, line)) {
        if (line.find(stockName) != std::string::npos) {
            ifile.close();
            return true;
        }
    }
    ifile.close();
    return false;
}
