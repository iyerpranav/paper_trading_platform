// Define the minimum Windows version to be Windows 10
#define _WIN32_WINNT 0x0A00

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"
#include "DatabaseManager.h"
#include "User.h"
#include <iostream>
#include <memory>

// Use nlohmann::json for convenience
using json = nlohmann::json;

const char* DB_FILE = "stock_portfolio.db";

int main() {
    // Initialize the database manager and server
    DatabaseManager dbManager(DB_FILE);
    if (!dbManager.initializeDatabase()) {
        std::cerr << "[FATAL] Could not initialize database. Exiting." << std::endl;
        return 1;
    }
    
    httplib::Server svr;

    // --- API Endpoints ---

    // POST /login
    svr.Post("/login", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[INFO] /login endpoint hit" << std::endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];
            int userId = -1;

            if (dbManager.validateUser(username, password, userId)) {
                json response_json = {
                    {"success", true},
                    {"userId", userId},
                    {"username", username}
                };
                res.set_content(response_json.dump(), "application/json");
            } else {
                res.status = 401; // Unauthorized
                json response_json = {{"success", false}, {"message", "Invalid credentials"}};
                res.set_content(response_json.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400; // Bad Request
            json response_json = {{"success", false}, {"message", e.what()}};
            res.set_content(response_json.dump(), "application/json");
        }
    });

    //POST /signin
    svr.Post("/signin", [&](const httplib::Request& req, httplib::Response& res){
        std::cout << "[INFO] /signin endpoint hit" << std::endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];
            std::string email = j["email"];
            int userId = -1;

            if (dbManager.addUser(username, password, email, userId)) {
                json response_json = {
                    {"success", true},
                    {"userId", userId},
                    {"username", username}
                };
                res.set_content(response_json.dump(), "application/json");
            } else {
                res.status = 401; // Unauthorized
                json response_json = {{"success", false}, {"message", "Invalid credentials"}};
                res.set_content(response_json.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400; // Bad Request
            json response_json = {{"success", false}, {"message", e.what()}};
            res.set_content(response_json.dump(), "application/json");
        }
    });

    // GET /portfolio/<userId>
    svr.Get(R"(/portfolio/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[INFO] /portfolio endpoint hit" << std::endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            int userId = std::stoi(req.matches[1]);
            Portfolio portfolio;
            dbManager.loadPortfolio(userId, portfolio);

            json portfolio_json;
            portfolio_json["fundBalance"] = portfolio.getFundBalance();
            json stocks_json = json::array();
            for (const auto& stock : portfolio.getStocks()) {
                stocks_json.push_back({
                    {"symbol", stock->getSymbol()},
                    {"quantity", stock->getQuantity()},
                    {"purchase_price", stock->getPurchasePrice()}
                });
            }
            portfolio_json["stocks"] = stocks_json;

            res.set_content(portfolio_json.dump(), "application/json");

        } catch (const std::exception& e) {
            res.status = 500;
            json response_json = {{"success", false}, {"message", e.what()}};
            res.set_content(response_json.dump(), "application/json");
        }
    });

    // POST /transaction
    svr.Post("/transaction", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[INFO] /transaction endpoint hit" << std::endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            auto j = json::parse(req.body);
            int userId = j["userId"];
            std::string type = j["type"];
            std::string symbol = j["symbol"];
            int quantity = j["quantity"];
            double price = j["price"];

            Portfolio portfolio;
            dbManager.loadPortfolio(userId, portfolio);
            
            bool success = false;
            if (type == "buy") {
                success = portfolio.buyStock(symbol, quantity, price);
            } else if (type == "sell") {
                success = portfolio.sellStock(symbol, quantity, price);
            }

            if (success) {
                dbManager.savePortfolio(userId, portfolio);
                json response_json = {{"success", true}};
                res.set_content(response_json.dump(), "application/json");
            } else {
                json response_json = {{"success", false}, {"message", "Transaction failed. Check funds or quantity."}};
                res.set_content(response_json.dump(), "application/json");
            }

        } catch (const std::exception& e) {
             res.status = 400;
             json response_json = {{"success", false}, {"message", e.what()}};
             res.set_content(response_json.dump(), "application/json");
        }
    });
    
    // GET /stocks
    svr.Get("/stocks", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[INFO] /stocks endpoint hit" << std::endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            json stocks_json = dbManager.getAllStocksAsJson();
            res.set_content(stocks_json.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            json response_json = {{"success", false}, {"message", e.what()}};
            res.set_content(response_json.dump(), "application/json");
        }
    });

    // POST /update_stocks
    svr.Post("/update_stocks", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[INFO] /update_stocks endpoint hit" << std::endl;
        res.set_header("Access-Control-Allow-Origin", "*");
        int returnCode = system("python stockdb.py");
        if (returnCode == 0) {
            res.set_content(R"({"success": true, "message": "Stock database updated."})", "application/json");
        } else {
            res.status = 500;
            res.set_content(R"({"success": false, "message": "Failed to execute Python script."})", "application/json");
        }
    });

    std::cout << "[INFO] Server started at http://localhost:8080" << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}

