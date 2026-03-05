// ============================================================
//  ThinkFast  |  src/core/GameEngine.cpp
//
//  Human turns call WordValidator::isValid() which uses:
//    - instant local dictionary check, then
//    - live Free Dictionary API call if not found locally
//  Bot turns use local dictionary only (instant).
// ============================================================

#include "GameEngine.h"
#include "../ui/ConsoleUI.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>

namespace ThinkFast {

GameEngine::GameEngine(WordValidator& wv)
    : wv_(wv), rng_(std::random_device{}()) {}

// ── Last-Letter mode ──────────────────────────────────────────

void GameEngine::runLastLetter(GameSession& sess) {
    sess.running = true;
    usedSet_.clear();

    std::string start = wv_.randomStartWord();
    sess.last_word        = start;
    sess.required_letter  = wv_.lastChar(start);
    sess.used_words.push_back(start);
    usedSet_.insert(start);

    while (sess.activePlayers() > 1 && sess.running) {
        Player* cur = sess.currentPlayer();
        if (!cur || cur->eliminated) { sess.nextPlayer(); continue; }

        renderLL(sess);

        bool ok = (cur->type == PlayerType::BOT)
                  ? doBotTurnLL(sess, *cur)
                  : doHumanTurnLL(sess, *cur);

        if (!ok) {
            cur->hearts--;
            ConsoleUI::error(cur->username + ": wrong or no word. -1 heart.");
            ConsoleUI::pause(1000);
            if (cur->hearts <= 0) {
                cur->eliminated = true;
                ConsoleUI::warn(cur->username + " has been eliminated.");
                ConsoleUI::pause(1200);
            }
        }
        sess.nextPlayer();
    }
    endGame(sess);
}

// ── One-by-One mode ───────────────────────────────────────────

void GameEngine::runOneByOne(GameSession& sess) {
    sess.running = true;
    sess.building.clear();

    while (sess.running) {
        Player* cur = sess.currentPlayer();
        if (!cur || cur->eliminated) { sess.nextPlayer(); continue; }

        renderOBO(sess);

        char ch = '\0';
        if (cur->type == PlayerType::BOT) {
            ch = wv_.cpuNextLetter(sess.building);
            ConsoleUI::pause(600);
        } else {
            ch = getHumanLetter(*cur);
        }

        if (!std::isalpha(static_cast<unsigned char>(ch))) {
            ConsoleUI::error("Letters only, please.");
            continue;
        }
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        const std::string candidate = sess.building + ch;

        // Does the candidate form a complete valid word?
        if (candidate.size() >= 3 && wv_.isValid(candidate)) {
            sess.building = candidate;
            renderOBO(sess);
            ConsoleUI::success("\"" + candidate + "\" — " + cur->username + " wins the round!");
            ConsoleUI::pause(2000);

            for (auto* p : sess.players) {
                if (!p->eliminated && p != cur) {
                    p->hearts--;
                    if (p->hearts <= 0) {
                        p->eliminated = true;
                        ConsoleUI::warn(p->username + " eliminated.");
                    }
                }
            }
            if (sess.activePlayers() <= 1) { sess.running = false; break; }
            sess.building.clear();
            sess.round++;
            ConsoleUI::pause(1000);

        } else if (!wv_.isValidPrefix(candidate)) {
            // No word starts with this string
            cur->hearts--;
            ConsoleUI::error(cur->username + ": \"" + candidate +
                             "\" has no valid continuation. -1 heart.");
            ConsoleUI::pause(1000);
            if (cur->hearts <= 0) {
                cur->eliminated = true;
                ConsoleUI::warn(cur->username + " eliminated.");
                if (sess.activePlayers() <= 1) { sess.running = false; break; }
            }
            sess.building.clear();
        } else {
            sess.building = candidate;
        }
        sess.nextPlayer();
    }
    endGame(sess);
}

// ── Rendering ─────────────────────────────────────────────────

void GameEngine::renderLL(const GameSession& sess) const {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::dhr(Color::BLUE);
    std::cout << Color::BOLD << Color::CYAN
              << "  MODE: Last Letter  |  Round " << sess.round
              << Color::RESET << "\n";
    ConsoleUI::hr();

    const Player* activePl = sess.players[sess.current_idx % sess.players.size()];
    for (const auto* p : sess.players) {
        const std::string hearts(p->hearts, 'v');
        std::cout << "  ";
        if (p == activePl)         std::cout << Color::YELLOW << Color::BOLD;
        else if (p->eliminated)    std::cout << Color::DARK_GRAY;
        else                       std::cout << Color::WHITE;
        std::cout << p->username
                  << (p->eliminated ? "  [OUT]" : "")
                  << Color::GREEN << "  [" << hearts << "]"
                  << Color::RESET << "\n";
    }

    ConsoleUI::hr();
    std::cout << Color::WHITE
              << "  Last word : " << Color::CYAN << Color::BOLD
              << sess.last_word << Color::RESET << "\n"
              << Color::WHITE
              << "  Start with: " << Color::YELLOW << Color::BOLD
              << static_cast<char>(std::toupper(
                     static_cast<unsigned char>(sess.required_letter)))
              << Color::RESET << "\n";
    ConsoleUI::dhr(Color::BLUE);
}

void GameEngine::renderOBO(const GameSession& sess) const {
    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::dhr(Color::BLUE);
    std::cout << Color::BOLD << Color::CYAN
              << "  MODE: One-by-One  |  Round " << sess.round
              << Color::RESET << "\n";
    ConsoleUI::hr();

    const Player* activePl = sess.players[sess.current_idx % sess.players.size()];
    for (const auto* p : sess.players) {
        const std::string hearts(p->hearts, 'v');
        std::cout << "  ";
        if (p == activePl)        std::cout << Color::YELLOW << Color::BOLD;
        else if (p->eliminated)   std::cout << Color::DARK_GRAY;
        else                      std::cout << Color::WHITE;
        std::cout << p->username
                  << (p->eliminated ? "  [OUT]" : "")
                  << Color::GREEN << "  [" << hearts << "]"
                  << Color::RESET << "\n";
    }

    ConsoleUI::hr();
    std::cout << Color::WHITE << "  Building: "
              << Color::CYAN << Color::BOLD
              << (sess.building.empty() ? "_" : sess.building)
              << Color::RESET << "\n";
    ConsoleUI::dhr(Color::BLUE);
}

void GameEngine::endGame(GameSession& sess) {
    Player* winner = nullptr;
    for (auto* p : sess.players)
        if (!p->eliminated) { winner = p; break; }

    for (auto* p : sess.players) {
        if (p->eliminated) p->losses++;
        else               p->wins++;
        p->games++;
    }

    ConsoleUI::clear();
    ConsoleUI::logo();
    ConsoleUI::dhr(Color::GREEN);
    if (winner) {
        std::cout << Color::BOLD << Color::GREEN
                  << ConsoleUI::center("GAME OVER  --  Winner: " + winner->username)
                  << "\n" << Color::RESET;
    } else {
        std::cout << Color::BOLD << Color::YELLOW
                  << ConsoleUI::center("GAME OVER  --  No winner")
                  << "\n" << Color::RESET;
    }
    ConsoleUI::dhr(Color::GREEN);
    ConsoleUI::pause(2000);
}

// ── Human turn: Last Letter ───────────────────────────────────

bool GameEngine::doHumanTurnLL(GameSession& sess, Player& player) {
    const int limit = sess.time_limit_secs;

    std::cout << Color::CYAN << "  " << player.username
              << ", your word [" << limit << "s]: "
              << Color::WHITE;
    std::cout.flush();

    std::string word = timedInput(limit);
    std::cout << Color::RESET << "\n";

    if (word.empty()) {
        ConsoleUI::error("Time up!");
        return false;
    }

    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    if (!wv_.startsWithChar(word, sess.required_letter)) {
        ConsoleUI::error("Must start with '" +
            std::string(1, static_cast<char>(
                std::toupper(static_cast<unsigned char>(sess.required_letter)))) + "'.");
        return false;
    }
    if (usedSet_.count(word)) {
        ConsoleUI::error("\"" + word + "\" was already used.");
        return false;
    }

    // Two-layer validation: local first, then API
    std::cout << Color::DARK_GRAY << "  Checking \"" << word << "\"..." << Color::RESET;
    std::cout.flush();

    if (!wv_.isValid(word)) {
        std::cout << "\n";
        ConsoleUI::error("\"" + word + "\" is not a recognised English word.");
        return false;
    }

    std::cout << Color::GREEN << " valid.\n" << Color::RESET;

    sess.last_word       = word;
    sess.required_letter = wv_.lastChar(word);
    sess.used_words.push_back(word);
    usedSet_.insert(word);
    return true;
}

// ── Bot turn: Last Letter ─────────────────────────────────────

bool GameEngine::doBotTurnLL(GameSession& sess, Player& bot) {
    std::uniform_int_distribution<int> delay(700, 1400);
    ConsoleUI::pause(delay(rng_));

    // Bots only use the local dictionary (no API latency)
    const std::string word = wv_.cpuPickWord(sess.required_letter, usedSet_, 1);
    if (word.empty()) {
        ConsoleUI::warn(bot.username + " [bot] could not find a word.");
        return false;
    }

    std::cout << Color::DARK_GRAY << "  " << bot.username << " [bot]: "
              << Color::WHITE << word << Color::RESET << "\n";
    ConsoleUI::pause(400);

    sess.last_word       = word;
    sess.required_letter = wv_.lastChar(word);
    sess.used_words.push_back(word);
    usedSet_.insert(word);
    return true;
}

// ── Human letter pick: One-by-One ────────────────────────────

char GameEngine::getHumanLetter(Player& player) {
    std::cout << Color::CYAN << "  " << player.username
              << ", type a letter: " << Color::WHITE;
    std::cout.flush();
    std::string in;
    std::getline(std::cin, in);
    std::cout << Color::RESET;
    return in.empty() ? '\0' : in[0];
}

// ── Timed terminal input ──────────────────────────────────────
//  Puts the terminal into non-canonical mode and polls for input.
//  Returns an empty string on timeout.

std::string GameEngine::timedInput(int seconds) {
    struct termios oldt{}, newt{};
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag     &= ~static_cast<tcflag_t>(ICANON);
    newt.c_cc[VMIN]   = 0;
    newt.c_cc[VTIME]  = 1;   // poll every 0.1 s
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string result;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(seconds);

    while (std::chrono::steady_clock::now() < deadline) {
        char c{};
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == '\n' || c == '\r') break;
            if (c == 127 || c == '\b') {
                if (!result.empty()) {
                    result.pop_back();
                    std::cout << "\b \b";
                    std::cout.flush();
                }
            } else if (std::isprint(static_cast<unsigned char>(c))) {
                result += c;
                std::cout << c;
                std::cout.flush();
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
}

} // namespace ThinkFast
