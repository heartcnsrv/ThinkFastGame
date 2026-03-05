#include "GameTypes.h"
#include <algorithm>

namespace ThinkFast {


std::string Player::statsStr() const {
    return "W:" + std::to_string(wins) +
           " L:" + std::to_string(losses) +
           " G:" + std::to_string(games);
}


Player* GameSession::currentPlayer() {
    if (players.empty()) return nullptr;
    return players[current_idx % static_cast<int>(players.size())];
}

void GameSession::nextPlayer() {
    const int n = static_cast<int>(players.size());
    if (n == 0) return;
    do {
        current_idx = (current_idx + 1) % n;
    } while (players[current_idx]->eliminated && activePlayers() > 0);
}

int GameSession::activePlayers() const {
    int count = 0;
    for (const auto* p : players)
        if (!p->eliminated) ++count;
    return count;
}

} 
