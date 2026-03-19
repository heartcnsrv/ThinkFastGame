#include "AuthManager.h"
#include <algorithm>
#include <ctime>

namespace ThinkFast {

AuthManager::AuthManager(const std::string& usersPath)
    : usersPath_(usersPath)
{
    // Load the current CSV snapshot as soon as the auth subsystem starts.
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

    // Registration appends a new logical row, then rewrites users.csv.
    records_.push_back(rec);
    CSVManager::saveUsers(usersPath_, records_);
    login(username, password);
    return true;
}

void AuthManager::saveStats(const Player& player) {
    if (player.is_guest) return;
    // Gameplay code updates Player in memory first. saveStats copies the
    // changed counters into the matching stored record and rewrites the CSV.
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
    // Leaderboard is derived from stored user records, not a separate table.
    std::sort(sorted.begin(), sorted.end(),
              [](const CSVManager::UserRecord& a,
                 const CSVManager::UserRecord& b){
                  return a.wins > b.wins;
              });
    return sorted;
}

void AuthManager::reload() {
    // HTTP mode can call reload() before each request to avoid serving stale
    // data if another request already changed the CSV file on disk.
    records_ = CSVManager::loadUsers(usersPath_);
}


void AuthManager::fillPlayer(const CSVManager::UserRecord& rec) {
    // Convert a storage record into the richer runtime Player structure.
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
