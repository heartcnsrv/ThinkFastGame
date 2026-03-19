// 
//  ThinkFast  |  main.cpp
//
//  Entry point.  Wires together:
//    WordValidator  — two-layer word check (local dict + API)
//    AuthManager    — login / register / guest / CSV persistence
//    GameEngine     — Last Letter and One-by-One game loops
//    Screens        — terminal UI screens
//
//  Word validation flow for human turns:
//    1. Instant check in built-in local dictionary
//    2. If not found locally -> curl call to:
//       https://api.dictionaryapi.dev/api/v2/entries/en/<word>
//       HTTP 200 = valid.  HTTP 404 = invalid.
//    Results are cached per session so the same word never hits
//    the API twice.
//
//  Build:
//    make          (uses the provided Makefile)
//    cmake ..      (uses the provided CMakeLists.txt)
// 

#include "src/core/WordValidator.h"
#include "src/core/AuthManager.h"
#include "src/core/GameEngine.h"
#include "src/ui/ConsoleUI.h"
#include "src/ui/Screens.h"
#include <iostream>
#include <filesystem>
#include <csignal>
#include <fstream>


static void onSignal(int) {
    ThinkFast::ConsoleUI::showCursor();
    std::cout << ThinkFast::Color::RESET << "\n";
    std::exit(0);
}

int main() {
    std::signal(SIGINT,  onSignal);
    std::signal(SIGTERM, onSignal);

    ThinkFast::ConsoleUI::hideCursor();

    namespace fs = std::filesystem;

    const std::string dataDir   = "data";
    const std::string usersPath = dataDir + "/users.csv";

    // The terminal build keeps its persistent "database" as a CSV file.
    // main.cpp ensures that storage exists before AuthManager reads from it.
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

    // High-level dependency flow:
    // users.csv -> AuthManager -> Screens
    // dictionary -> WordValidator -> GameEngine/Screens
    // GameEngine updates Player stats -> AuthManager persists them to CSV.
    ThinkFast::AuthManager auth(usersPath);
    ThinkFast::GameEngine  engine(validator);
    ThinkFast::Screens     screens(auth, engine, validator);

    while (true) {
        screens.runLogin();
        if (auth.isLoggedIn())
            screens.runMainMenu();
    }
}
