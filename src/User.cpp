#include "D:/Projects/stock_portfolio/include/User.h"

void User::authenticate() {
    std::cout << "Welcome to the platform!" << std::endl;
    int choice;
    do {
        std::cout << "Menu:\n1. Register\n2. Login" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            std::cout << "Enter a username: "; 
            std::cin >> username;
            if (validateUser()) {
                std::cout << "\tUsername already exists. Try again" << std::endl;
                continue;
            }
            std::cout << "Enter a password: "; 
            std::cin >> password;
            std::ofstream file("data/user_db.txt", std::ios::app);
            file << username << " " << password << " " << std::endl; 
            file.close();
            std::cout << "User registered successfully!" << std::endl;
            continue;
        } 
    } while (true);
}
