#ifndef USER_H
#define USER_H

#include "Portfolio.h"
#include <string>
#include <memory>

/**
 * @class User
 * @brief Represents an application user.
 *
 * This class handles user authentication and holds the user's portfolio.
 */
class User {
public:
    /**
     * @brief Constructs a new User object.
     * @param username The user's username.
     * @param userId The user's unique ID from the database.
     */
    User(const std::string& username, int userId);

    /**
     * @brief Gets the username.
     * @return The user's username.
     */
    std::string getUsername() const;
    
    /**
     * @brief Gets the user's ID.
     * @return The user's ID.
     */
    int getUserId() const;

    /**
     * @brief Provides access to the user's portfolio.
     * @return A reference to the Portfolio object.
     */
    Portfolio& getPortfolio();

private:
    std::string username;
    int userId;
    std::unique_ptr<Portfolio> portfolio;
};

#endif // USER_H

