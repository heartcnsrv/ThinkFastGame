#pragma once

#include "GameTypes.h"
#include "../utils/CSVManager.h"
#include <string>
#include <vector>

namespace ThinkFast {

/*
 * Authentication and account service.
 * Login, registration, stat saving, and leaderboard reads
 * go through this class instead of touching users.csv directly.
 */
class AuthManager {
public:
    explicit AuthManager(const std::string& usersPath);

    /* Checks username/password against the loaded CSV records. */
    Player* login(const std::string& username, const std::string& password);

    /* Creates a temporary guest player that is not persisted to CSV. */
    Player* loginGuest(const std::string& name);

    /* Adds a new user account if the username is still available. */
    bool registerUser(const std::string& username, const std::string& password);

    /* Copies updated player stats into storage and rewrites the CSV file. */
    void saveStats(const Player& player);

    /* Returns true when a player session is currently active. */
    bool isLoggedIn() const;
    /* Returns the currently logged-in player object. */
    Player* currentPlayer();
    /* Clears the current login state. */
    void logout();
    /* Checks whether the username already exists in users.csv. */
    bool userExists(const std::string& username) const;

    /* Returns all stored users sorted for leaderboard display. */
    std::vector<CSVManager::UserRecord> leaderboard() const;

    /* Reloads the latest CSV snapshot from disk. */
    void reload();

private:
    std::string                         usersPath_;
    std::vector<CSVManager::UserRecord> records_; /* In-memory users.csv snapshot. */
    Player                              current_; /* Active session player. */
    bool                                loggedIn_ = false; /* Session flag. */

    /* Copies one stored record into the runtime Player model. */
    void fillPlayer(const CSVManager::UserRecord& rec);
    /* Builds today's date string for new account creation. */
    static std::string todayString();
};

}
