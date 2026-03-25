#pragma once

#include <string>
#include <vector>

namespace ThinkFast {

/*
 * Core game enums used across terminal mode and web mode.
 * These values describe which game rules are active
 * and what kind of participant is taking a turn.
 */
enum class GameMode   { LAST_LETTER, ONE_BY_ONE };
enum class PlayerType { HUMAN, BOT, REMOTE };

/*
 * Shared player model used by authentication, terminal gameplay,
 * and multiplayer room logic.
 * Some fields come from users.csv, while others exist only
 * during the current match.
 */
struct Player {
    std::string  username;            /* Player login name or guest name. */
    std::string  password;            /* Stored password for CSV-backed accounts. */
    int          wins        = 0;     /* Total wins saved to persistent storage. */
    int          losses      = 0;     /* Total losses saved to persistent storage. */
    int          games       = 0;     /* Total played matches. */
    std::string  joined_date;         /* Account creation date as YYYY-MM-DD. */
    int          hearts      = 3;     /* Remaining lives in the current match. */
    bool         eliminated  = false; /* True when the player is out of the round. */
    PlayerType   type        = PlayerType::HUMAN; /* Human, bot, or remote player. */
    bool         is_guest    = false; /* Guest accounts are not saved to CSV. */

    /* Builds a compact terminal summary of the player's record. */
    std::string statsStr() const;
};

/*
 * In-memory state container for one terminal match.
 * This struct is temporary runtime data only.
 * Only the finished Player statistics are saved after the game ends.
 */
struct GameSession {
    GameMode                 mode            = GameMode::LAST_LETTER; /* Active ruleset. */
    std::vector<Player*>     players;                                /* Turn order list. */
    int                      current_idx     = 0;                    /* Current turn index. */
    std::string              last_word;                              /* Last accepted word. */
    char                     required_letter = '\0';                 /* Next starting letter. */
    std::vector<std::string> used_words;                             /* Prevents reuse. */
    int                      round           = 1;                    /* Current round number. */
    int                      time_limit_secs = 15;                   /* Human turn timer. */
    bool                     running         = false;                /* Match loop flag. */
    std::string              building;                               /* One-by-One partial word. */

    /* Returns the player whose turn is currently active. */
    Player* currentPlayer();
    /* Advances to the next non-eliminated player when possible. */
    void    nextPlayer();
    /* Counts how many players are still active in the match. */
    int     activePlayers() const;
};

}
