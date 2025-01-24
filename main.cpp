#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

class Stock {
protected:
    void updateStockInfo() {
        int returnCode = system("python stockdb.py");
        if (returnCode == 0) {
            cout << "\n[INFO] Python stock program executed successfully." << endl;
        } else {
            cerr << "\n[ERROR] Failed to execute Python stock program." << endl;
        }
    }
   
    void displayInfo(int option = 0) {
        ifstream ifile("stock_info.csv");
        if (ifile.fail()) {
            cerr << "\n[ERROR] Could not open stock_info.csv." << endl;
            return;
        }

        cout << "\n======================= STOCK DATA =======================\n\n";
        string line;
        char delimiter = ',';
        cout << left << setw(15) << "Symbol" << setw(20) << "Prev Close" << setw(30) << "Day Range";
        if (option == 1) {
            cout << setw(30) << "Year Range" << setw(20) << "Market Cap" << setw(15) << "Avg Volume";
            cout << setw(10) << "Div Yield" << setw(10) << "P/E Ratio";
        }
        cout << "\n" << string(100, '-') << "\n";

        while (getline(ifile, line)) {
            vector<string> tokens;
            stringstream ss(line);
            string token;
            while (getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 8) {
                cout << left << setw(15) << tokens[0] << setw(20) << tokens[1] << setw(30) << tokens[2];
                if (option == 1) {
                    cout << setw(30) << tokens[3] << setw(20) << tokens[4] << setw(15) << tokens[5];
                    cout << setw(10) << tokens[6] << setw(10) << tokens[7];
                }
                cout << endl;
            } else {
                cerr << "[WARNING] Invalid line format: " << line << endl;
            }
        }
        cout << string(100, '=') << "\n";
        ifile.close();
    }

    bool validateStockName(const string& stockName, const string& filename) {
        ifstream ifile(filename);
        if (ifile.fail()) {
            cerr << "\n[ERROR] Could not open " << filename << "." << endl;
            return false;
        }

        string line;
        string lowerStockName = stockName;
        transform(lowerStockName.begin(), lowerStockName.end(), lowerStockName.begin(), ::tolower); 

        while (getline(ifile, line)) {
            string lowerLine = line;
            transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
            if (lowerLine.find(lowerStockName) != string::npos) {
                ifile.close();
                return true; 
            }
        }

        ifile.close();
        return false;
    }
};

class Portfolio : protected Stock {
protected:
    double fundBalance = 10000.00;
    string stockName;
    int quantity;

    vector<vector<string>> loadPortfolio(const string& filename) {
        vector<vector<string>> data;
        ifstream ifile(filename);
        if (ifile.fail()) {
            cerr << "\n[ERROR] Could not open " << filename << "." << endl;
            return data;
        }
        
        string line;
        char delimiter = ',';
        while (getline(ifile, line)) {
            stringstream ss(line);
            string token;
            vector<string> tokens;
            while (getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }
            if (!tokens.empty()) {
                data.push_back(tokens);
            }
        }
        ifile.close();
        return data;
    }

    void savePortfolio(const vector<vector<string>>& data, const string& filename) {
        ofstream ofile(filename);
        if (ofile.fail()) {
            cerr << "\n[ERROR] Could not write to " << filename << "." << endl;
            return;
        }
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                ofile << row[i];
                if (i < row.size() - 1) ofile << ",";
            }
            ofile << endl;
        }
        ofile.close();
    }

    void displayPortfolio() {
        auto portfolioData = loadPortfolio("portfolio.csv");
        if (portfolioData.empty()) {
            cout << "\n[INFO] Your portfolio is empty.\n";
            return;
        }

        cout << "\n======================= PORTFOLIO =======================\n";
        cout << left << setw(15) << "Stock" << setw(10) << "Qty" << setw(15) << "Buy Price\n";
        cout << string(40, '-') << "\n";

        for (const auto& row : portfolioData) {
            if (row.size() >= 3) {
                cout << left << setw(15) << row[0] << setw(10) << row[1] << setw(15) << row[2] << endl;
            }
        }
        cout << string(40, '=') << "\n";
    }

    void executeBuyTransaction() {
        cout << "\n[INFO] Your Current Fund Balance: $" << fixed << setprecision(2) << fundBalance << endl; 
        cout << "Enter the name of the stock you want to buy: "; 
        cin >> stockName;

        if (!validateStockName(stockName, "stock_info.csv")) {
            cerr << "\n[ERROR] Stock not found. Please try again." << endl;
            return;
        }

        cout << "Enter the quantity of shares you want to buy: "; 
        if (!(cin >> quantity) || quantity <= 0) {
            cerr << "\n[ERROR] Invalid quantity. Enter a positive number." << endl;
            return;
        }

        double buyPrice;
        cout << "Enter the buy price: $"; 
        if (!(cin >> buyPrice) || buyPrice <= 0) {
            cerr << "\n[ERROR] Invalid price. Enter a positive number." << endl;
            return;
        }

        double totalCost = buyPrice * quantity;
        if (totalCost > fundBalance) {
            cerr << "\n[ERROR] Insufficient funds to buy " << quantity << " shares of " << stockName << "." << endl;
            return;
        }

        fundBalance -= totalCost;
        auto portfolioData = loadPortfolio("portfolio.csv");
        bool stockExists = false;

        for (auto& row : portfolioData) {
            if (row[0] == stockName) {
                int currentQty = stoi(row[1]);
                row[1] = to_string(currentQty + quantity);
                row[2] = to_string(((currentQty * stod(row[2])) + totalCost) / (currentQty + quantity));
                stockExists = true;
                break;
            }
        }

        if (!stockExists) {
            portfolioData.push_back({stockName, to_string(quantity), to_string(buyPrice)});
        }

        savePortfolio(portfolioData, "portfolio.csv");
        cout << "\n[SUCCESS] Bought " << quantity << " shares of " << stockName << " at $" << buyPrice << " each.\n";
    }

    void executeSellTransaction() {
        cout << "\n[INFO] Your Current Fund Balance: $" << fundBalance << endl; 
        cout << "Enter the name of the stock you want to sell: "; 
        cin >> stockName;

        cout << "Enter the quantity of shares you want to sell: "; 
        if (!(cin >> quantity) || quantity <= 0) {
            cerr << "\n[ERROR] Invalid quantity. Enter a positive number." << endl;
            return;
        }

        double sellPrice;
        cout << "Enter the sell price: $"; 
        if (!(cin >> sellPrice) || sellPrice <= 0) {
            cerr << "\n[ERROR] Invalid price. Enter a positive number." << endl;
            return;
        }

        auto portfolioData = loadPortfolio("portfolio.csv");
        bool stockFound = false;

        for (auto& row : portfolioData) {
            if (row[0] == stockName) {
                int currentQty = stoi(row[1]);
                if (currentQty < quantity) {
                    cerr << "\n[ERROR] Insufficient shares to sell.\n";
                    return;
                }
                row[1] = to_string(currentQty - quantity);
                fundBalance += sellPrice * quantity;
                stockFound = true;
                break;
            }
        }

        if (!stockFound) {
            cerr << "\n[ERROR] Stock not found in your portfolio.\n";
            return;
        }

        savePortfolio(portfolioData, "portfolio.csv");
        cout << "\n[SUCCESS] Sold " << quantity << " shares of " << stockName << " at $" << sellPrice << " each.\n";
    }
};

class User : protected Portfolio {
private:
    string username, password;

    bool validateUser() {
        ifstream file("user_db.txt");
        if (file.fail()) {
            cerr << "\n[ERROR] Could not open user_db.txt." << endl;
            return false;
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string user, pass;
            getline(ss, user, ',');
            getline(ss, pass, ',');

            if (user == username && pass == password) {
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    void loginUser() {
        cout << "\n================ LOGIN ====================\n";
        int attempts = 0;
        while (attempts < 3) {
            cout << "Enter Username: ";
            cin >> username;
            cout << "Enter Password: ";
            cin >> password;

            if (validateUser()) {
                cout << "\n[INFO] Welcome, " << username << "!" << endl;
                return;
            } else {
                cerr << "\n[ERROR] Invalid credentials.\n";
                attempts++;
            }
        }
        cerr << "\n[ERROR] Maximum login attempts exceeded.\n";
        exit(EXIT_FAILURE);
    }

    void displayMainMenu() {
        while (true) {
            cout << "\n================ MAIN MENU =================\n";
            cout << "1. Update Stock Info\n";
            cout << "2. Display Stock Info\n";
            cout << "3. Display Portfolio\n";
            cout << "4. Buy Stock\n";
            cout << "5. Sell Stock\n";
            cout << "0. Exit\n";
            cout << "Enter your choice: ";
            int choice;
            cin >> choice;

            switch (choice) {
            case 1: updateStockInfo(); break;
            case 2: displayInfo(); break;
            case 3: displayPortfolio(); break;
            case 4: executeBuyTransaction(); break;
            case 5: executeSellTransaction(); break;
            case 0: cout << "\n[INFO] Exiting program. Goodbye!\n"; exit(EXIT_SUCCESS);
            default: cerr << "\n[ERROR] Invalid choice. Try again.\n"; break;
            }
        }
    }

public:
    User() {
        loginUser();
        displayMainMenu();
    }
};

int main() {
    User user;
    return 0;
}
