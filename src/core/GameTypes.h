#pragma once

#include <string>
#include <vector>

namespace ThinkFast {

enum class GameMode   { LAST_LETTER, ONE_BY_ONE };
enum class PlayerType { HUMAN, BOT, REMOTE };

// Player is the shared user/game model used across the project.
// AuthManager maps its persistent fields to CSV records, while game code
// mutates the runtime counters during a session.
struct Player {
    std::string  username;
    std::string  password;
    int          wins        = 0;
    int          losses      = 0;
    int          games       = 0;
    std::string  joined_date;
    int          hearts      = 3;
    bool         eliminated  = false;
    PlayerType   type        = PlayerType::HUMAN;
    bool         is_guest    = false;

    std::string statsStr() const;
};

// GameSession is the in-memory state container for one terminal match.
// It is not stored directly in the database; only Player stats survive
// after callers hand finished players back to AuthManager.
struct GameSession {
    GameMode             mode            = GameMode::LAST_LETTER;
    std::vector<Player*> players;
    int                  current_idx     = 0;
    std::string          last_word;
    char                 required_letter = '\0';
    std::vector<std::string> used_words;
    int                  round           = 1;
    int                  time_limit_secs = 15;
    bool                 running         = false;
    std::string          building;

    Player* currentPlayer();
    void    nextPlayer();
    int     activePlayers() const;
};

} 
