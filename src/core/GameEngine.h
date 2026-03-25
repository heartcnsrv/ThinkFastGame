#pragma once

#include "GameTypes.h"
#include "WordValidator.h"
#include <random>
#include <unordered_set>

namespace ThinkFast {

/*
 * Terminal gameplay engine.
 * This class runs the match rules and turn flow for the console version.
 * It stays separate from persistence so AuthManager remains
 * the only service that writes to users.csv.
 */
class GameEngine {
public:
    explicit GameEngine(WordValidator& wv);

    /* Runs one complete Last Letter match. */
    void runLastLetter(GameSession& session);
    /* Runs one complete One-by-One match. */
    void runOneByOne(GameSession& session);

private:
    WordValidator&                  wv_;      /* Shared word validation service. */
    std::mt19937                    rng_;     /* Random source for game choices. */
    std::unordered_set<std::string> usedSet_; /* Fast duplicate-word checks. */

    /* Draws the Last Letter game screen. */
    void renderLL(const GameSession& sess) const;
    /* Draws the One-by-One game screen. */
    void renderOBO(const GameSession& sess) const;
    /* Finalizes the match and updates runtime stats. */
    void endGame(GameSession& sess);

    /* Processes one human Last Letter turn. */
    bool doHumanTurnLL(GameSession& sess, Player& player);
    /* Processes one bot Last Letter turn. */
    bool doBotTurnLL(GameSession& sess, Player& bot);
    /* Reads one letter from a human player for One-by-One mode. */
    char getHumanLetter(Player& player);

    /* Waits for timed console input and returns the captured text. */
    std::string timedInput(int seconds);
};

}
