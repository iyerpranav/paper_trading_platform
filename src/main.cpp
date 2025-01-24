#include "D:/Projects/stock_portfolio/include/User.h"

int main() {
    User user;
    int choice;
    user.authenticate();

    do {
        std::cout << "\nMenu:\n1. View Stock Data\n2. Buy stocks\n3. Sell Stock\n4. View Portfolio\n5. Exit" << std::endl;
        std::cout << "Enter your choice: "; 
        std::cin >> choice;

        switch (choice) {
            case 1: user.viewStockData(); break;
            case 2: user.buyStock(); break;
            case 3: user.sellStock(); break;
            case 4: user.viewPortfolio(); break;
            case 5: break;
            default: std::cout << "Invalid choice. Try again." << std::endl; break;
        }
    } while (choice != 5);

    return 0;
}
