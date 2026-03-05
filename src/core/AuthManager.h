#pragma once

#include "GameTypes.h"
#include "../utils/CSVManager.h"
#include <vector>
#include <string>

namespace ThinkFast {

class AuthManager {
public:
    explicit AuthManager(const std::string& usersPath);

    Player* login(const std::string& username, const std::string& password);

    Player* loginGuest(const std::string& name);

    bool registerUser(const std::string& username, const std::string& password);

    void saveStats(const Player& player);

    bool    isLoggedIn()    const;
    Player* currentPlayer();
    void    logout();
    bool    userExists(const std::string& username) const;

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

}
