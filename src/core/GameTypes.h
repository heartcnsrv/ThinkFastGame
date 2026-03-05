#pragma once
// ============================================================
//  ThinkFast  |  src/core/GameTypes.h
//  Pure type declarations — no implementation here
// ============================================================

#include <string>
#include <vector>

namespace ThinkFast {

enum class GameMode   { LAST_LETTER, ONE_BY_ONE };
enum class PlayerType { HUMAN, BOT, REMOTE };

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

} // namespace ThinkFast
