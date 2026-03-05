#pragma once
// ============================================================
//  ThinkFast  |  src/core/AuthManager.h
// ============================================================

#include "GameTypes.h"
#include "../utils/CSVManager.h"
#include <vector>
#include <string>

namespace ThinkFast {

class AuthManager {
public:
    explicit AuthManager(const std::string& usersPath);

    // Returns pointer to logged-in player on success, nullptr on failure
    Player* login(const std::string& username, const std::string& password);

    // Guest session (not saved to CSV)
    Player* loginGuest(const std::string& name);

    // Register a new account; returns false if username taken or fields invalid
    bool registerUser(const std::string& username, const std::string& password);

    // Persist updated stats for a real (non-guest) player
    void saveStats(const Player& player);

    bool    isLoggedIn()    const;
    Player* currentPlayer();
    void    logout();
    bool    userExists(const std::string& username) const;

    // Returns players sorted by wins (descending) — no passwords exposed
    std::vector<CSVManager::UserRecord> leaderboard() const;

    void reload();

private:
    std::string                          usersPath_;
    std::vector<CSVManager::UserRecord>  records_;
    Player                               current_;
    bool                                 loggedIn_ = false;

    void        fillPlayer(const CSVManager::UserRecord& rec);
    static std::string todayString();
};

} // namespace ThinkFast
