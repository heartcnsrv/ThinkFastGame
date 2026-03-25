/*
 * Terminal application entry point.
 * This file prepares the CSV storage,
 * creates the core services,
 * and starts the console login and menu flow.
 *
 * Main dependency flow:
 * users.csv -> AuthManager -> Screens
 * dictionary -> WordValidator -> GameEngine and Screens
 * GameEngine updates Player stats -> AuthManager saves them to CSV
 */

#include "src/core/WordValidator.h"
#include "src/core/AuthManager.h"
#include "src/core/GameEngine.h"
#include "src/ui/ConsoleUI.h"
#include "src/ui/Screens.h"
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>

static void onSignal(int) {
    ThinkFast::ConsoleUI::showCursor();
    std::cout << ThinkFast::Color::RESET << "\n";
    std::exit(0);
}

int main() {
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    ThinkFast::ConsoleUI::hideCursor();

    namespace fs = std::filesystem;

    const std::string dataDir   = "data";
    const std::string usersPath = dataDir + "/users.csv";

    /* Create the local CSV storage first so AuthManager can read it safely. */
    fs::create_directories(dataDir);

    if (!fs::exists(usersPath)) {
        std::ofstream f(usersPath);
        f << "\"username\",\"password\",\"wins\",\"losses\","
             "\"games_played\",\"joined_date\"\n"
             "\"heart\",\"123\",\"0\",\"0\",\"0\",\"2025-01-01\"\n"
             "\"angel\",\"123\",\"0\",\"0\",\"0\",\"2025-01-01\"\n";
    }

    ThinkFast::ConsoleUI::clear();
    ThinkFast::ConsoleUI::logo();
    ThinkFast::ConsoleUI::info("Loading dictionary...");

    ThinkFast::WordValidator validator;

    ThinkFast::ConsoleUI::success("Dictionary ready ("
        + std::to_string(validator.dictionarySize()) + " words).");
    ThinkFast::ConsoleUI::info("Word API: api.dictionaryapi.dev (used for unknown words).");
    ThinkFast::ConsoleUI::pause(900);

    ThinkFast::AuthManager auth(usersPath);
    ThinkFast::GameEngine  engine(validator);
    ThinkFast::Screens     screens(auth, engine, validator);

    while (true) {
        screens.runLogin();
        if (auth.isLoggedIn()) {
            screens.runMainMenu();
        }
    }
}
