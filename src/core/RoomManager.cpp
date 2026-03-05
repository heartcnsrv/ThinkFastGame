// ============================================================
//  ThinkFast  |  src/core/RoomManager.cpp
//
//  Every game rule lives here. PHP never sees game state.
//  Rules are identical to GameEngine.cpp:
//    - 3 hearts per player
//    - wrong word / timeout / dead prefix → -1 heart → eliminated
//    - last player alive wins
//    - WordValidator::isValid() for all human word checks
//    - WordValidator::cpuPickWord / cpuNextLetter for bots
// ============================================================

#include "RoomManager.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <stdexcept>

namespace ThinkFast {

// ── Constructor ───────────────────────────────────────────────

RoomManager::RoomManager(WordValidator& wv)
    : wv_(wv), rng_(std::random_device{}()) {}

// ── Helpers ───────────────────────────────────────────────────

long long RoomManager::nowSec() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

int RoomManager::activePlayers(const Room& r) const {
    int n = 0;
    for (const auto& p : r.players) if (!p.eliminated) ++n;
    return n;
}

void RoomManager::advanceTurn(Room& r) {
    const int n = static_cast<int>(r.players.size());
    if (n == 0) return;
    for (int i = 0; i < n; ++i) {
        r.current_idx = (r.current_idx + 1) % n;
        if (!r.players[r.current_idx].eliminated) break;
    }
    r.turn_started = nowSec();

    // If next player is a bot, play its turn immediately
    auto& cur = r.players[r.current_idx];
    if (!cur.is_bot || cur.eliminated) return;

    if (r.mode == GameMode::LAST_LETTER) {
        std::unordered_set<std::string> used(r.used_words.begin(), r.used_words.end());
        std::string word = wv_.cpuPickWord(r.required_letter, used, 1);
        if (word.empty()) {
            cur.hearts--;
            r.log.push_back(cur.name + " [bot] couldn't find a word. -1 heart.");
            if (cur.hearts <= 0) {
                cur.eliminated = true;
                r.log.push_back(cur.name + " [bot] eliminated.");
            }
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);
        } else {
            r.log.push_back(cur.name + " [bot]: " + word);
            r.last_word       = word;
            r.required_letter = wv_.lastChar(word);
            r.used_words.insert(word);
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);
        }
    } else {
        char letter    = wv_.cpuNextLetter(r.building);
        std::string candidate = r.building + letter;
        r.log.push_back(cur.name + " [bot] adds: " + letter + " -> " + candidate);

        if (candidate.size() >= 3 && wv_.isValidLocal(candidate)) {
            r.building = candidate;
            r.log.push_back(cur.name + " [bot] completed: \"" + candidate + "\"");
            for (auto& p : r.players) {
                if (!p.eliminated && p.name != cur.name) {
                    p.hearts--;
                    r.log.push_back(p.name + " -1 heart.");
                    if (p.hearts <= 0) { p.eliminated = true; r.log.push_back(p.name + " eliminated."); }
                }
            }
            r.building = "";
            r.round++;
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);
        } else if (!wv_.isValidPrefix(candidate)) {
            cur.hearts--;
            r.log.push_back(cur.name + " [bot]: \"" + candidate + "\" dead end. -1 heart.");
            if (cur.hearts <= 0) { cur.eliminated = true; r.log.push_back(cur.name + " eliminated."); }
            r.building = "";
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);
        } else {
            r.building = candidate;
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);
        }
    }
}

void RoomManager::checkWin(Room& r) {
    if (activePlayers(r) <= 1) r.status = RoomStatus::FINISHED;
}

// ── JSON helpers ─────────────────────────────────────────────

static std::string jsonEscape(const std::string& s) {
    std::string o;
    for (unsigned char c : s) {
        switch (c) {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n";  break;
            case '\r': o += "\\r";  break;
            case '\t': o += "\\t";  break;
            default:   o += static_cast<char>(c); break;
        }
    }
    return o;
}

std::string RoomManager::ok(const std::string& body) const {
    if (body.empty()) return "{\"ok\":true}";
    return "{\"ok\":true," + body + "}";
}

std::string RoomManager::err(const std::string& msg) const {
    return "{\"ok\":false,\"error\":\"" + jsonEscape(msg) + "\"}";
}

std::string RoomManager::roomToJson(const Room& r,
                                     const std::string& forPlayer) const {
    // status string
    std::string st = (r.status == RoomStatus::WAITING)  ? "waiting"
                   : (r.status == RoomStatus::PLAYING)  ? "playing"
                   :                                       "finished";

    // players array
    std::string players = "[";
    for (size_t i = 0; i < r.players.size(); ++i) {
        const auto& p = r.players[i];
        if (i) players += ",";
        players += "{\"name\":\"" + jsonEscape(p.name) + "\","
                   "\"hearts\":"  + std::to_string(p.hearts)     + ","
                   "\"eliminated\":" + (p.eliminated ? "true" : "false") + ","
                   "\"is_bot\":"  + (p.is_bot ? "true" : "false") + "}";
    }
    players += "]";

    // log array (last 50)
    std::string logArr = "[";
    const int start = std::max(0, static_cast<int>(r.log.size()) - 50);
    for (int i = start; i < static_cast<int>(r.log.size()); ++i) {
        if (i > start) logArr += ",";
        logArr += "\"" + jsonEscape(r.log[i]) + "\"";
    }
    logArr += "]";

    // used_words array
    std::string used = "[";
    bool first = true;
    for (const auto& w : r.used_words) {
        if (!first) used += ",";
        first = false;
        used += "\"" + jsonEscape(w) + "\"";
    }
    used += "]";

    // is_your_turn
    bool iyt = (r.status == RoomStatus::PLAYING) &&
               !r.players.empty() &&
               (r.players[r.current_idx % r.players.size()].name == forPlayer);

    std::string modeStr = (r.mode == GameMode::LAST_LETTER) ? "last_letter" : "one_by_one";

    std::string json =
        "\"room\":{"
        "\"code\":\""           + jsonEscape(r.code)            + "\","
        "\"mode\":\""           + modeStr                        + "\","
        "\"status\":\""         + st                             + "\","
        "\"host\":\""           + jsonEscape(r.host)             + "\","
        "\"time_limit\":"       + std::to_string(r.time_limit)   + ","
        "\"max_players\":"      + std::to_string(r.max_players)  + ","
        "\"players\":"          + players                        + ","
        "\"current_idx\":"      + std::to_string(r.current_idx)  + ","
        "\"last_word\":\""      + jsonEscape(r.last_word)         + "\","
        "\"required_letter\":\"" + std::string(1, r.required_letter) + "\","
        "\"building\":\""       + jsonEscape(r.building)          + "\","
        "\"round\":"            + std::to_string(r.round)         + ","
        "\"used_words\":"       + used                            + ","
        "\"log\":"              + logArr                          + "},"
        "\"is_your_turn\":"     + (iyt ? "true" : "false");

    return json;
}

// ── Code generator ────────────────────────────────────────────

std::string RoomManager::generateCode() const {
    static const char CHARS[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::uniform_int_distribution<int> d(0, 31);
    std::string code;
    for (int i = 0; i < 6; ++i) code += CHARS[d(rng_)];
    return code;
}

// ── CREATE ────────────────────────────────────────────────────

std::string RoomManager::create(const std::string& player,
                                 const std::string& modeStr,
                                 int timeLimit, int maxPlayers) {
    std::lock_guard<std::mutex> lk(mu_);

    Room r;
    do { r.code = generateCode(); } while (rooms_.count(r.code));

    r.mode        = (modeStr == "one_by_one") ? GameMode::ONE_BY_ONE
                                               : GameMode::LAST_LETTER;
    r.time_limit  = std::max(5, std::min(60, timeLimit));
    r.max_players = std::max(2, std::min(8, maxPlayers));
    r.host        = player;
    r.status      = RoomStatus::WAITING;
    r.created_at  = nowSec();

    RoomPlayer rp;
    rp.name      = player;
    rp.last_seen = nowSec();
    r.players.push_back(rp);

    std::string code = r.code;
    rooms_[code] = std::move(r);
    return ok(roomToJson(rooms_[code], player) +
              ",\"code\":\"" + code + "\"");
}

// ── JOIN ──────────────────────────────────────────────────────

std::string RoomManager::join(const std::string& player,
                               const std::string& code) {
    std::lock_guard<std::mutex> lk(mu_);

    auto it = rooms_.find(code);
    if (it == rooms_.end())              return err("Room not found.");
    Room& r = it->second;
    if (r.status != RoomStatus::WAITING) return err("Game already started.");

    // Already in?
    for (auto& p : r.players)
        if (p.name == player) { p.last_seen = nowSec(); return ok(roomToJson(r, player)); }

    if (static_cast<int>(r.players.size()) >= r.max_players)
        return err("Room is full.");

    RoomPlayer rp;
    rp.name      = player;
    rp.last_seen = nowSec();
    r.players.push_back(rp);

    return ok(roomToJson(r, player));
}

// ── STATE (poll) ──────────────────────────────────────────────

std::string RoomManager::state(const std::string& player,
                                const std::string& code) {
    std::lock_guard<std::mutex> lk(mu_);

    auto it = rooms_.find(code);
    if (it == rooms_.end()) return err("Room not found.");
    Room& r = it->second;

    // Refresh last_seen
    for (auto& p : r.players)
        if (p.name == player) { p.last_seen = nowSec(); break; }

    // Auto-kick disconnected players (15s without a poll)
    if (r.status == RoomStatus::PLAYING) {
        const long long now = nowSec();
        for (auto& p : r.players) {
            if (!p.eliminated && !p.is_bot && (now - p.last_seen) > 15) {
                p.eliminated = true;
                p.hearts     = 0;
                r.log.push_back(p.name + " disconnected and was eliminated.");
            }
        }
        checkWin(r);
    }

    return ok(roomToJson(r, player));
}

// ── START ─────────────────────────────────────────────────────

std::string RoomManager::start(const std::string& player,
                                const std::string& code) {
    std::lock_guard<std::mutex> lk(mu_);

    auto it = rooms_.find(code);
    if (it == rooms_.end())               return err("Room not found.");
    Room& r = it->second;
    if (r.host != player)                 return err("Only the host can start.");
    if (r.status != RoomStatus::WAITING)  return err("Game already started.");
    if (static_cast<int>(r.players.size()) < 2) return err("Need at least 2 players.");

    r.status      = RoomStatus::PLAYING;
    r.current_idx = 0;
    r.turn_started = nowSec();

    if (r.mode == GameMode::LAST_LETTER) {
        std::string start = wv_.randomStartWord();
        r.last_word       = start;
        r.required_letter = wv_.lastChar(start);
        r.used_words.insert(start);
        r.log.push_back("Game started. First word: " + start);
    } else {
        r.building = "";
        r.log.push_back("Game started. One-by-One mode.");
    }

    // If first player is a bot, advance immediately
    if (!r.players.empty() && r.players[0].is_bot) {
        advanceTurn(r);
    }

    return ok(roomToJson(r, player));
}

// ── MOVE ──────────────────────────────────────────────────────

std::string RoomManager::move(const std::string& player,
                               const std::string& code,
                               const std::string& value) {
    std::lock_guard<std::mutex> lk(mu_);

    auto it = rooms_.find(code);
    if (it == rooms_.end())                return err("Room not found.");
    Room& r = it->second;
    if (r.status != RoomStatus::PLAYING)   return err("Game is not in progress.");

    // Validate it's this player's turn
    const int n = static_cast<int>(r.players.size());
    if (n == 0) return err("No players.");
    auto& cur = r.players[r.current_idx % n];
    if (cur.name != player)                return err("Not your turn.");
    if (cur.eliminated)                    return err("You are eliminated.");

    // Sanitise value
    std::string val = value;
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    for (char c : val) if (!std::isalpha(static_cast<unsigned char>(c))) return err("Letters only.");
    if (val.empty()) return err("No value provided.");

    if (r.mode == GameMode::LAST_LETTER) {
        // ── Last Letter rules (mirrors GameEngine::doHumanTurnLL) ──

        if (val[0] != r.required_letter)
            return err(std::string("Word must start with '") +
                       static_cast<char>(std::toupper(
                           static_cast<unsigned char>(r.required_letter))) + "'.");

        if (r.used_words.count(val))
            return err("\"" + val + "\" was already used.");

        // WordValidator::isValid — local dict first, then real dictionary API
        if (!wv_.isValid(val))
            return err("\"" + val + "\" is not a recognised English word.");

        r.log.push_back(player + ": " + val);
        r.last_word       = val;
        r.required_letter = wv_.lastChar(val);
        r.used_words.insert(val);
        r.turn_started    = nowSec();

        checkWin(r);
        if (r.status == RoomStatus::PLAYING) advanceTurn(r);

    } else {
        // ── One-by-One rules (mirrors GameEngine::runOneByOne) ──

        if (val.size() != 1)
            return err("Submit exactly one letter.");

        const std::string candidate = r.building + val;
        r.log.push_back(player + " adds: " + val + " -> " + candidate);

        if (candidate.size() >= 3 && wv_.isValid(candidate)) {
            // Player completed a valid word — everyone else -1 heart
            r.building = candidate;
            r.log.push_back(player + " completed: \"" + candidate + "\"");
            for (auto& p : r.players) {
                if (!p.eliminated && p.name != player) {
                    p.hearts--;
                    r.log.push_back(p.name + " -1 heart.");
                    if (p.hearts <= 0) {
                        p.eliminated = true;
                        r.log.push_back(p.name + " eliminated.");
                    }
                }
            }
            r.building = "";
            r.round++;
            r.turn_started = nowSec();
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);

        } else if (!wv_.isValidPrefix(candidate)) {
            // Dead end — current player -1 heart
            cur.hearts--;
            r.log.push_back(player + ": \"" + candidate + "\" has no valid continuation. -1 heart.");
            if (cur.hearts <= 0) {
                cur.eliminated = true;
                r.log.push_back(player + " eliminated.");
            }
            r.building = "";
            r.turn_started = nowSec();
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);

        } else {
            // Still a valid prefix — keep building
            r.building    = candidate;
            r.turn_started = nowSec();
            checkWin(r);
            if (r.status == RoomStatus::PLAYING) advanceTurn(r);
        }
    }

    return ok(roomToJson(r, player));
}

// ── LEAVE ─────────────────────────────────────────────────────

std::string RoomManager::leave(const std::string& player,
                                const std::string& code) {
    std::lock_guard<std::mutex> lk(mu_);

    auto it = rooms_.find(code);
    if (it == rooms_.end()) return ok();

    Room& r = it->second;
    for (auto& p : r.players) {
        if (p.name == player) {
            p.eliminated = true;
            p.hearts     = 0;
            break;
        }
    }
    r.log.push_back(player + " left the game.");
    checkWin(r);

    return ok();
}

} // namespace ThinkFast
