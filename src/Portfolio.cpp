#include "D:/Projects/stock_portfolio/include/Portfolio.h"

void Portfolio::updatePortfolio() {
    std::thread t1([](){
        int returnCode = system("python scripts/portfolio.py");
        if (returnCode == 0) {
            std::cout << "\tPython portfolio program executed successfully." << std::endl;
        } else {
            std::cerr << "Error executing Python portfolio program." << std::endl;
        }
    });
    t1.join();
}

void Portfolio::displayPortfolio() {
    std::ifstream ifile("data/portfolio.csv");
    if (ifile.fail()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    std::cout << "\n---PORTFOLIO DATA---\tfund balance " << fundBalance << "\n\n";
    std::string line;
    while (getline(ifile, line)) {
        std::cout << line << std::endl;
    }
    ifile.close();
}
