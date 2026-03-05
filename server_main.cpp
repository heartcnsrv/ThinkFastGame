//
//  ThinkFast  |  server_main.cpp
//
//  C++ HTTP backend — compile on Windows with MinGW:
//
//  g++ -std=c++17 -O2 -o ThinkFastServer.exe ^
//      server_main.cpp ^
//      src/core/RoomManager.cpp ^
//      src/server/HttpServer.cpp ^
//      src/core/GameTypes.cpp ^
//      src/core/WordValidator.cpp ^
//      src/core/AuthManager.cpp ^
//      src/utils/CSVManager.cpp ^
//      src/utils/BotNames.cpp ^
//      -lws2_32
//
//  Then run:  ThinkFastServer.exe
// 

#include "src/core/AuthManager.h"
#include "src/core/WordValidator.h"
#include "src/core/RoomManager.h"
#include "src/server/HttpServer.h"
#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include <sys/stat.h> 

#ifdef _WIN32
  #include <direct.h> 
  #define MAKE_DIR(p) _mkdir(p)
#else
  #include <unistd.h>
  #define MAKE_DIR(p) mkdir(p, 0755)
#endif

static ThinkFast::HttpServer* g_server = nullptr;

static void onSignal(int) {
    std::cout << "\n[ThinkFast] Shutting down...\n";
    if (g_server) g_server->stop();
    std::exit(0);
}

static bool fileExists(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  onSignal);
    std::signal(SIGTERM, onSignal);

    int port = 8080;
    if (argc > 1) {
        try { port = std::stoi(argv[1]); } catch (...) {}
    }

    MAKE_DIR("data");

    const std::string usersPath = "data/users.csv";
    if (!fileExists(usersPath)) {
        std::ofstream f(usersPath);
        f << "\"username\",\"password\",\"wins\",\"losses\","
             "\"games_played\",\"joined_date\"\n"
             "\"heart\",\"123\",\"0\",\"0\",\"0\",\"2025-01-01\"\n"
             "\"angel\",\"123\",\"0\",\"0\",\"0\",\"2025-01-01\"\n";
        std::cout << "[ThinkFast] Created data/users.csv with demo accounts.\n";
    }

    // ── Initialise subsystems ────────────────────────────────────
    std::cout << "[ThinkFast] Loading dictionary...\n";
    ThinkFast::WordValidator validator;
    std::cout << "[ThinkFast] Dictionary ready ("
              << validator.dictionarySize() << " words).\n";

    ThinkFast::AuthManager auth(usersPath);
    ThinkFast::RoomManager rooms(validator);
    std::cout << "[ThinkFast] Auth + RoomManager ready.\n";

    // ── Start HTTP server ────────────────────────────────────────
    ThinkFast::HttpServer server(port, auth, validator, rooms);
    g_server = &server;

    std::cout << "[ThinkFast] Server starting on port " << port << "\n"
              << "[ThinkFast] Endpoints:\n"
              << "   POST http://localhost:" << port << "/auth\n"
              << "   POST http://localhost:" << port << "/validate\n"
              << "   POST http://localhost:" << port << "/leaderboard\n"
              << "   POST http://localhost:" << port << "/room\n"
              << "[ThinkFast] Press Ctrl+C to stop.\n\n";

    server.run();
    return 0;
}
