// ============================================================
//  ThinkFast  |  src/ui/Screens.cpp
// ============================================================

#include "Screens.h"
#include "ConsoleUI.h"
#include "../utils/BotNames.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace ThinkFast {

Screens::Screens(AuthManager& auth, GameEngine& engine, WordValidator& wv)
    : auth_(auth), engine_(engine), wv_(wv) {}

// ── Login screen ──────────────────────────────────────────────

void Screens::runLogin() {
    while (!auth_.isLoggedIn()) {
        ConsoleUI::clear();
        ConsoleUI::logo();

        const int choice = ConsoleUI::menu(
            "Welcome! Choose an option:",
            { "Login", "Register", "Play as Guest", "Quit" });

        switch (choice) {
            case 0: doLogin();    break;
            case 1: doRegister(); break;
            case 2: doGuest();    break;
            case 3: ConsoleUI::showCursor(); std::exit(0);
        }
    }
}

void Screens::doLogin() {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::println(Color::BOLD + std::string("  Sign In") + Color::RESET);
    ConsoleUI::hr();

    const std::string user = ConsoleUI::prompt("Username");
    const std::string pass = ConsoleUI::prompt("Password");

    if (auth_.login(user, pass)) {
        ConsoleUI::success("Welcome back, " + user + "!");
        ConsoleUI::pause(800);
    } else {
        ConsoleUI::error("Invalid username or password.");
        ConsoleUI::pause(1200);
    }
}

void Screens::doRegister() {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::println(Color::BOLD + std::string("  Create Account") + Color::RESET);
    ConsoleUI::hr();

    const std::string user  = ConsoleUI::prompt("Username");
    const std::string pass  = ConsoleUI::prompt("Password");
    const std::string pass2 = ConsoleUI::prompt("Confirm password");

    if (pass != pass2) {
        ConsoleUI::error("Passwords do not match.");
        ConsoleUI::pause(1200);
        return;
    }
    if (user.size() < 2) {
        ConsoleUI::error("Username must be at least 2 characters.");
        ConsoleUI::pause(1200);
        return;
    }
    if (pass.size() < 3) {
        ConsoleUI::error("Password must be at least 3 characters.");
        ConsoleUI::pause(1200);
        return;
    }
    if (auth_.registerUser(user, pass)) {
        ConsoleUI::success("Account created. You are now logged in as " + user + ".");
        ConsoleUI::pause(800);
    } else {
        ConsoleUI::error("Username \"" + user + "\" is already taken.");
        ConsoleUI::pause(1200);
    }
}

void Screens::doGuest() {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::warn("Guest stats are not saved.");
    ConsoleUI::hr();

    const std::string name = ConsoleUI::prompt("Display name");
    auth_.loginGuest(name.empty() ? "Guest" : name);
    ConsoleUI::success("Playing as guest: " + (name.empty() ? "Guest" : name));
    ConsoleUI::pause(700);
}

// ── Main menu ─────────────────────────────────────────────────

void Screens::runMainMenu() {
    while (true) {
        ConsoleUI::clear();
        ConsoleUI::logo();

        Player* me = auth_.currentPlayer();
        if (me) {
            std::cout << Color::DARK_GRAY << "  Logged in as: "
                      << Color::WHITE << me->username;
            if (!me->is_guest)
                std::cout << Color::DARK_GRAY << "  (" << me->statsStr() << ")";
            std::cout << Color::RESET << "\n\n";
        }

        const int choice = ConsoleUI::menu(
            "Main Menu",
            { "Play vs Bots", "Leaderboard", "Log Out" });

        switch (choice) {
            case 0: runLobby();       break;
            case 1: runLeaderboard(); break;
            case 2:
                auth_.logout();
                return;
        }
    }
}

// ── Lobby ─────────────────────────────────────────────────────

void Screens::runLobby() {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::println(Color::BOLD + std::string("  Game Setup") + Color::RESET);
    ConsoleUI::hr();

    // Mode
    const int modeChoice = ConsoleUI::menu(
        "Select game mode:",
        { "Last Letter  (each word starts with the last letter of the previous)",
          "One-by-One   (take turns adding one letter at a time)" });
    const GameMode mode = (modeChoice == 0) ? GameMode::LAST_LETTER
                                             : GameMode::ONE_BY_ONE;

    // Bot count
    const int botChoice = ConsoleUI::menu("Number of bot opponents:", { "1", "2", "3" });
    const int botCount  = botChoice + 1;

    // Time limit (Last Letter only)
    int timeLimit = 15;
    if (mode == GameMode::LAST_LETTER) {
        const int tChoice = ConsoleUI::menu(
            "Time limit per turn:",
            { "10 seconds", "15 seconds", "20 seconds", "30 seconds" });
        const int times[] = { 10, 15, 20, 30 };
        timeLimit = times[tChoice];
    }

    // Build players list
    Player* me = auth_.currentPlayer();
    std::vector<Player> botPlayers(botCount);
    for (auto& b : botPlayers) {
        b.username = BotNames::random();
        b.type     = PlayerType::BOT;
        b.hearts   = 3;
    }

    GameSession sess;
    sess.mode            = mode;
    sess.time_limit_secs = timeLimit;
    sess.players.push_back(me);
    for (auto& b : botPlayers) sess.players.push_back(&b);

    // Run
    if (mode == GameMode::LAST_LETTER)
        engine_.runLastLetter(sess);
    else
        engine_.runOneByOne(sess);

    // Save stats
    if (me && !me->is_guest)
        auth_.saveStats(*me);

    ConsoleUI::pause(1000);
    ConsoleUI::prompt("Press Enter to return to menu");
}

// ── Leaderboard ───────────────────────────────────────────────

void Screens::runLeaderboard() {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::println(Color::BOLD + std::string("  Leaderboard") + Color::RESET);
    ConsoleUI::dhr(Color::BLUE);

    auto rows = auth_.leaderboard();
    const int TOP = 15;

    // Header
    std::cout << Color::BOLD << Color::BLUE
              << ConsoleUI::pad("  #",  5)
              << ConsoleUI::pad("Player",    20)
              << ConsoleUI::pad("Wins",  8)
              << ConsoleUI::pad("Losses", 9)
              << ConsoleUI::pad("Games",  7)
              << Color::RESET << "\n";
    ConsoleUI::hr();

    for (int i = 0; i < std::min(TOP, static_cast<int>(rows.size())); ++i) {
        const auto& r = rows[i];
        const char* rankCol = (i == 0) ? Color::YELLOW
                            : (i == 1) ? Color::WHITE
                            : (i == 2) ? Color::TEAL
                            :            Color::GRAY;
        std::cout << rankCol
                  << ConsoleUI::pad("  " + std::to_string(i + 1), 5)
                  << ConsoleUI::pad(r.username,         20)
                  << ConsoleUI::pad(std::to_string(r.wins),    8)
                  << ConsoleUI::pad(std::to_string(r.losses),  9)
                  << ConsoleUI::pad(std::to_string(r.games_played), 7)
                  << Color::RESET << "\n";
    }

    ConsoleUI::dhr(Color::BLUE);
    ConsoleUI::prompt("Press Enter to go back");
}

} // namespace ThinkFast
