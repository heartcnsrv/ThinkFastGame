#pragma once

#include "GameTypes.h"
#include "WordValidator.h"
#include <unordered_set>
#include <random>

namespace ThinkFast {

class GameEngine {
public:
    explicit GameEngine(WordValidator& wv);

    void runLastLetter(GameSession& session);
    void runOneByOne(GameSession& session);

private:
    WordValidator&                   wv_;
    std::mt19937                     rng_;
    std::unordered_set<std::string>  usedSet_;

    void renderLL(const GameSession& sess) const;
    void renderOBO(const GameSession& sess) const;
    void endGame(GameSession& sess);

    bool doHumanTurnLL(GameSession& sess, Player& player);
    bool doBotTurnLL(GameSession& sess, Player& bot);
    char getHumanLetter(Player& player);

    std::string timedInput(int seconds);
};

}
