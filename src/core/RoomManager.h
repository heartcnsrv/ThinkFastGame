#pragma once

#include "GameTypes.h"
#include "WordValidator.h"
#include "../utils/BotNames.h"
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ThinkFast {

/* Multiplayer room state categories used by the HTTP backend. */
enum class RoomStatus { WAITING, PLAYING, FINISHED };

/*
 * Player snapshot inside a multiplayer room.
 * This is session data only, not permanent account data.
 */
struct RoomPlayer {
    std::string name;              /* Display name inside the room. */
    int         hearts     = 3;    /* Remaining lives for this match. */
    bool        eliminated = false; /* True when removed from active play. */
    long long   last_seen  = 0;    /* Heartbeat timestamp for disconnect checks. */
    bool        is_bot     = false; /* True for automated room players. */
};

/*
 * Live multiplayer room state.
 * The server keeps this in memory while it is running
 * and exposes it through the /room endpoint.
 */
struct Room {
    std::string              code;            /* Short join code. */
    GameMode                 mode            = GameMode::LAST_LETTER; /* Active ruleset. */
    int                      time_limit      = 15;  /* Turn timer in seconds. */
    int                      max_players     = 4;   /* Room capacity. */
    RoomStatus               status          = RoomStatus::WAITING; /* Match phase. */
    std::string              host;            /* Name of the room creator. */
    std::vector<RoomPlayer>  players;         /* All current participants. */
    int                      current_idx     = 0;   /* Active turn position. */
    std::string              last_word;       /* Last accepted Last Letter word. */
    char                     required_letter = '\0'; /* Needed starting letter. */
    std::unordered_set<std::string> used_words; /* Prevents duplicate words. */
    std::string              building;        /* Shared One-by-One fragment. */
    int                      round           = 1;   /* Current room round. */
    long long                turn_started    = 0;   /* Turn start timestamp. */
    std::vector<std::string> log;             /* Simple room event history. */
    long long                created_at      = 0;   /* Room creation timestamp. */
};

/*
 * Multiplayer room service behind /room requests.
 * It keeps rooms in memory and serializes access with a mutex
 * so concurrent web requests do not corrupt shared room state.
 */
class RoomManager {
public:
    explicit RoomManager(WordValidator& wv);

    /* Creates a new room and returns a JSON response. */
    std::string create(const std::string& player, const std::string& mode,
                       int timeLimit, int maxPlayers);
    /* Adds a player to an existing room. */
    std::string join(const std::string& player, const std::string& code);
    /* Returns the latest room snapshot for one player. */
    std::string state(const std::string& player, const std::string& code);
    /* Starts a waiting room once the host requests it. */
    std::string start(const std::string& player, const std::string& code);
    /* Applies one move and advances the room state. */
    std::string move(const std::string& player, const std::string& code,
                     const std::string& value);
    /* Removes a player from the room. */
    std::string leave(const std::string& player, const std::string& code);

private:
    WordValidator&                        wv_;    /* Shared word service. */
    std::unordered_map<std::string, Room> rooms_; /* All active rooms by code. */
    mutable std::mutex                    mu_;    /* Protects rooms_. */
    mutable std::mt19937                  rng_;   /* Random room-code source. */

    /* Generates a short unique room code. */
    std::string generateCode() const;
    /* Serializes one room into the JSON shape expected by clients. */
    std::string roomToJson(const Room& r, const std::string& forPlayer) const;
    /* Wraps a successful JSON response. */
    std::string ok(const std::string& body = "") const;
    /* Wraps an error JSON response. */
    std::string err(const std::string& msg) const;

    /* Moves control to the next eligible room player. */
    void advanceTurn(Room& r);
    /* Detects whether the room already has a winner. */
    void checkWin(Room& r);
    /* Counts the non-eliminated players still active. */
    int activePlayers(const Room& r) const;
    /* Returns the current Unix timestamp in seconds. */
    static long long nowSec();
};

}
