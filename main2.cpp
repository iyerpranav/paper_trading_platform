#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>

using namespace std;

class Stock {
protected:
    void updateStockInfo() {
        thread t1([](){
            int returnCode = system("python stockdb.py");
            if (returnCode == 0) {
                cout << "\tPython stock program executed successfully." << endl;
            } else {
                cerr << "Error executing Python stock program." << endl;
            }
        });
        t1.join();
    }
   
    void displayInfo(int option = 0) {
        ifstream ifile("stock_info.csv");
        if (ifile.fail()) {
            cerr << "Error opening file." << endl;
            return;
        }

        cout << "\n-----------------------STOCK DATA-----------------------\n\n";
        string line;
        char delimiter = ',';
        while (getline(ifile, line)) {
            vector<string> tokens;
            stringstream ss(line);
            string token;
            while (getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 8) {
                if (option == 0) {
                    cout << left << setw(15) << tokens[0];   // Symbol
                    cout << left << setw(20) << tokens[1];   // Previous close
                    cout << left << setw(30) << tokens[2];   // Day range
                    cout << endl;
                } else {
                    cout << left << setw(30) << tokens[3];   // Year range
                    cout << left << setw(20) << tokens[4];   // Market cap
                    cout << left << setw(15) << tokens[5];   // Avg Volume
                    cout << left << setw(10) << tokens[6];   // Dividend yield
                    cout << left << setw(10) << tokens[7];   // P/E ratio
                    cout << endl;
                }
            } else {
                cout << "Invalid line format: " << line << endl;
            }
        }
        ifile.close();
    }

    bool validateStockName(const string& stockName, const string& filename) {
        ifstream ifile(filename);
        if (ifile.fail()) {
            cerr << "Error opening file." << endl;
            return false;
        }

        string line;
        while (getline(ifile, line)) {
            if (line.find(stockName) != string::npos) {
                ifile.close();
                return true; // Stock name exists
            }
        }

        ifile.close();
        return false;
    }
};

class Portfolio {
protected:
    double fundBalance = 10000.00, stockPrice, buyPrice, sellPrice;
    double totalCost, totalValue, profLoss;
    int holdQty;
    string stockName;
    int quantity;
    void updatePortfolio() {
        thread t1([](){
            int returnCode = system("python portfolio.py");
            if (returnCode == 0) {
                cout << "\tPython portfolio program executed successfully." << endl;
            } else {
                cerr << "Error executing Python portfolio program." << endl;
            }
        });
        t1.join();
    }

    void displayPortfolio() {
        ifstream ifile("portfolio.csv");
        if (ifile.fail()) {
            cerr << "Error opening file." << endl;
            return;
        }

        cout << "\n---PORTFOLIO DATA---\t\t\t\t\t\t\tfund balance " << fundBalance << "\n\n";
        string line;
        char delimiter = ',';
        while (getline(ifile, line)) {
            vector<string> tokens;
            stringstream ss(line);
            string token;
            while (getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 7) {
                cout << "| " << left << setw(15) << tokens[0] << " |";   // Symbol
                cout << "| " << left << setw(15) << tokens[1] << " |";   // Previous close
                cout << "| " << left << setw(20) << tokens[2] << " |";   // Day range
                cout << "| " << left << setw(20) << tokens[3] << " |";   // Year range
                cout << "| " << left << setw(15) << tokens[4] << " |";   // Market cap
                cout << "| " << left << setw(15) << tokens[5] << " |";   // Avg Volume
                cout << "| " << left << setw(15) << tokens[6] << " |";   // Dividend yield
                cout << endl;
            } else {
                cout << "Invalid line format: " << line << endl;
            }
        }
        ifile.close();
    }
    
    void getBuySellInfo(const string& name, int option = 0) {
        ifstream ifile(name);
        if (ifile.fail()) {
            cerr << "Error opening file." << endl;
            return;
        }

        string line;
        while (getline(ifile, line)) {
            stringstream ss(line);
            string token;
            vector<string> tokens;

            while (getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 1 && tokens[0] == stockName) {
                if (option == 1)
                    holdQty = stoi(tokens[1]);
                else
                    stockPrice = stod(tokens[1]);
            }
        }
        ifile.close();
    }
    
    void executeBuyTransaction() {
        cout << "Your Fund balance is: "<<fundBalance; 
        cout << "\nEnter the quantity of shares you want to buy: "; 
        cin >> quantity;
        cout << "Enter the buy price: "; 
        cin >> buyPrice;
        
        getBuySellInfo("stock_db.csv");
        totalCost = buyPrice * quantity;
        totalValue = stockPrice * quantity;
        profLoss = totalValue - totalCost;
        
        if (totalCost < fundBalance) {
            fundBalance -= totalCost;
            ofstream ofile("portfolio.csv", ios::app);

            string line = stockName + ',' + to_string(quantity) + ',' + to_string(buyPrice) + ',' +
                               to_string(stockPrice) + ',' + to_string(totalCost) + ',' +
                               to_string(totalValue) + ',' + to_string(profLoss);
            ofile << endl << line;
            ofile.close();

            cout << "Successfully bought " << quantity << " shares of " << stockName << "." << endl;
            cout << "\nYour Fund balance is: "<<fundBalance; 

        } else {
            cout << "Insufficient funds to buy " << quantity << " shares of " << stockName << "." << endl;
            return;
        }
    }
    
    void executeSellTransaction() {
        cout << "Your Fund balance is: "<<fundBalance; 
        cout << "\nEnter the quantity of shares you want to sell: "; 
        cin >> quantity;
        getBuySellInfo("portfolio.csv", 1);

        if (holdQty > quantity) {
            cout << "Enter the sell price: "; 
            cin >> sellPrice;
            fundBalance += sellPrice * quantity;
            holdQty -= quantity;
            totalCost = buyPrice * holdQty;
            totalValue = stockPrice * holdQty;
            profLoss = totalValue - totalCost;

            ofstream ofile("portfolio.csv", ios::app);

            string line = stockName + ',' + to_string(holdQty) + ',' + to_string(buyPrice) + ',' +
                               to_string(stockPrice) + ',' + to_string(totalCost) + ',' +
                               to_string(totalValue) + ',' + to_string(profLoss);
            ofile << endl << line;
            ofile.close();

            cout << "Successfully sold " << quantity << " shares of " << stockName << "." << endl;
            cout << "\nYour Fund balance is: "<<fundBalance; 
        } else {
            cout << "Insufficient quantity to sell " << quantity << " shares of " << stockName << "." << endl;
        }
    }
};

class User : protected Stock, protected Portfolio {
private:
    string username, password;
    bool validateUser() {
        ifstream file("user_db.txt");
        if (file.fail()) {
            cerr << "Error opening file." << endl;
            return false;
        }

        string line;
        while (getline(file, line)) {
            if (line.find(username) != string::npos) {
                file.close();
                return true; // Username exists
            }
        }
        file.close();
        return false;
    }

public:

    void authenticate() {
        cout << "Welcome to the platform!" << endl;
        int choice;
        do {
            cout << "Menu:\n1. Register\n2. Login" << endl;
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 1) {
                cout << "Enter a username: "; 
                cin >> username;
                if (validateUser()) {
                    cout << "\tUsername already exists. Try again" << endl;
                    continue;
                }
                cout << "Enter a password: "; 
                cin >> password;
                ofstream file("user_db.txt", ios::app);
                file << username << " " << password << " " << endl; 
                file.close();
                cout << "User registered successfully!" << endl;
                continue;
            } else if (choice == 2) {
                cout << "Enter your username: "; 
                cin >> username;
                cout << "Enter your password: "; 
                cin >> password;
                ifstream file("user_db.txt");
                string line;
                while (getline(file, line)) {
                    string storedUsername = line.substr(0, line.find(' '));
                    string storedPassword = line.substr(line.find(' ') + 1);
                    if (username == storedUsername && password == storedPassword) {
                        cout << "\nLogin successful!";
                        file.close();
                        return;
                    }
                }
                file.close();
                cout << "\tInvalid username or password. Please try again.";
            }
        } while (true);
    }

    void viewStockData() {
        displayInfo();
    }

    void viewStockTrend() {
        displayInfo(2);
    }

    void viewPortfolio() {
        displayPortfolio();
    }

    void buyStock() {
        cout << "Enter the name of the stock you want to buy: "; 
        cin >> stockName;
        if (validateStockName(stockName, "stock_info.csv"))
            executeBuyTransaction();
        else
            cout << "\tInvalid stock name. Please try again." << endl;
    }
    
    void sellStock() {
        cout << "Enter the name of the stock you want to sell: "; 
        cin >> stockName;
        if (validateStockName(stockName, "portfolio.csv"))
            executeSellTransaction();
        else
            cout << "\tInvalid stock name. Please try again." << endl;
    }
};


int main() {
    User user;
    int choice;
    user.authenticate();

    do {
        cout<<"\n--------------------------------------";
        cout << "\nMenu:\n1. View Stock Data\n2. View Stock Trend\n3. Buy stocks\n4. Sell Stock\n5. View Portfolio\n6. Exit" << endl;
        cout << "--------------------------------------\nEnter your choice: "; 
        cin >> choice;

        switch (choice) {
            case 1:
                user.viewStockData();
                break;
            case 2:
                user.viewStockTrend();
                break;
            case 3:
                user.buyStock();
                break;
            case 4:
                user.sellStock();
                break;
            case 5:
                user.viewPortfolio();
                break;
            case 6:
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
        }
    } while (choice != 6);

    return 0;
}
