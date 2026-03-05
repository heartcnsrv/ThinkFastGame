#pragma once
// ============================================================
//  ThinkFast  |  src/core/RoomManager.h
// ============================================================

#include "GameTypes.h"
#include "WordValidator.h"
#include "../utils/BotNames.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <random>

namespace ThinkFast {

enum class RoomStatus { WAITING, PLAYING, FINISHED };

struct RoomPlayer {
    std::string name;
    int         hearts     = 3;
    bool        eliminated = false;
    long long   last_seen  = 0;
    bool        is_bot     = false;
};

struct Room {
    std::string              code;
    GameMode                 mode            = GameMode::LAST_LETTER;
    int                      time_limit      = 15;
    int                      max_players     = 4;
    RoomStatus               status          = RoomStatus::WAITING;
    std::string              host;
    std::vector<RoomPlayer>  players;
    int                      current_idx     = 0;
    std::string              last_word;
    char                     required_letter = '\0';
    std::unordered_set<std::string> used_words;
    std::string              building;
    int                      round           = 1;
    long long                turn_started    = 0;
    std::vector<std::string> log;
    long long                created_at      = 0;
};

class RoomManager {
public:
    explicit RoomManager(WordValidator& wv);

    std::string create(const std::string& player, const std::string& mode,
                       int timeLimit, int maxPlayers);
    std::string join  (const std::string& player, const std::string& code);
    std::string state (const std::string& player, const std::string& code);
    std::string start (const std::string& player, const std::string& code);
    std::string move  (const std::string& player, const std::string& code,
                       const std::string& value);
    std::string leave (const std::string& player, const std::string& code);

private:
    WordValidator&                        wv_;
    std::unordered_map<std::string, Room> rooms_;
    mutable std::mutex                    mu_;
    mutable std::mt19937                  rng_;

    std::string generateCode() const;
    std::string roomToJson(const Room& r, const std::string& forPlayer) const;
    std::string ok (const std::string& body = "") const;
    std::string err(const std::string& msg)  const;

    void advanceTurn    (Room& r);
    void checkWin       (Room& r);
    int  activePlayers  (const Room& r) const;
    static long long nowSec();
};

} // namespace ThinkFast
