#pragma once
// ============================================================
//  ThinkFast  |  src/ui/Screens.h
// ============================================================

#include "../core/AuthManager.h"
#include "../core/GameEngine.h"
#include "../core/WordValidator.h"

namespace ThinkFast {

class Screens {
public:
    Screens(AuthManager& auth, GameEngine& engine, WordValidator& wv);

    void runLogin();
    void runMainMenu();
    void runLobby();
    void runLeaderboard();

private:
    AuthManager&   auth_;
    GameEngine&    engine_;
    WordValidator& wv_;

    void doLogin();
    void doRegister();
    void doGuest();
};

} // namespace ThinkFast
