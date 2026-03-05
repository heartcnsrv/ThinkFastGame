#include "AuthManager.h"
#include <algorithm>
#include <ctime>

namespace ThinkFast {

AuthManager::AuthManager(const std::string& usersPath)
    : usersPath_(usersPath)
{
    reload();
}

Player* AuthManager::login(const std::string& username,
                           const std::string& password) {
    for (const auto& rec : records_) {
        if (rec.username == username && rec.password == password) {
            fillPlayer(rec);
            loggedIn_ = true;
            return &current_;
        }
    }
    return nullptr;
}

Player* AuthManager::loginGuest(const std::string& name) {
    current_             = Player{};
    current_.username    = name.empty() ? "Guest" : name;
    current_.is_guest    = true;
    current_.type        = PlayerType::HUMAN;
    loggedIn_            = true;
    return &current_;
}

bool AuthManager::registerUser(const std::string& username,
                               const std::string& password) {
    if (username.empty() || password.empty()) return false;
    if (userExists(username))                 return false;

    CSVManager::UserRecord rec;
    rec.username    = username;
    rec.password    = password;
    rec.joined_date = todayString();

    records_.push_back(rec);
    CSVManager::saveUsers(usersPath_, records_);
    login(username, password);
    return true;
}

void AuthManager::saveStats(const Player& player) {
    if (player.is_guest) return;
    for (auto& rec : records_) {
        if (rec.username == player.username) {
            rec.wins         = player.wins;
            rec.losses       = player.losses;
            rec.games_played = player.games;
            break;
        }
    }
    CSVManager::saveUsers(usersPath_, records_);
}

bool AuthManager::isLoggedIn() const { return loggedIn_; }

Player* AuthManager::currentPlayer() {
    return loggedIn_ ? &current_ : nullptr;
}

void AuthManager::logout() { loggedIn_ = false; }

bool AuthManager::userExists(const std::string& username) const {
    for (const auto& r : records_)
        if (r.username == username) return true;
    return false;
}

std::vector<CSVManager::UserRecord> AuthManager::leaderboard() const {
    auto sorted = records_;
    std::sort(sorted.begin(), sorted.end(),
              [](const CSVManager::UserRecord& a,
                 const CSVManager::UserRecord& b){
                  return a.wins > b.wins;
              });
    return sorted;
}

void AuthManager::reload() {
    records_ = CSVManager::loadUsers(usersPath_);
}


void AuthManager::fillPlayer(const CSVManager::UserRecord& rec) {
    current_             = Player{};
    current_.username    = rec.username;
    current_.password    = rec.password;
    current_.wins        = rec.wins;
    current_.losses      = rec.losses;
    current_.games       = rec.games_played;
    current_.joined_date = rec.joined_date;
    current_.type        = PlayerType::HUMAN;
    current_.is_guest    = false;
}

std::string AuthManager::todayString() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}

}
