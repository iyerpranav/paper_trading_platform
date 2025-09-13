#include "User.h"

User::User(const std::string& username, int userId) 
    : username(username), userId(userId) {
    // Initialize portfolio with a default balance.
    portfolio = std::make_unique<Portfolio>(10000.00); 
}

std::string User::getUsername() const {
    return username;
}

int User::getUserId() const {
    return userId;
}

Portfolio& User::getPortfolio() {
    return *portfolio;
}

