// ============================================================
//  ThinkFast  |  src/server/HttpServer.cpp
//
//  Cross-platform HTTP/1.1 server.
//  Windows: Winsock2   (winsock2.h / ws2_32.lib)
//  Linux:   POSIX      (sys/socket.h)
//
//  No external libraries. Pure sockets + threads.
//  All game logic lives in AuthManager / WordValidator / RoomManager.
// ============================================================

#include "HttpServer.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
  #pragma comment(lib, "ws2_32.lib")
#endif

namespace ThinkFast {

static std::mutex authMu;

// ── sendAll: keep sending until entire buffer is flushed ──────
static void sendAll(sock_t fd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        int n = ::send(fd,
                       data.c_str() + sent,
                       static_cast<int>(data.size() - sent),
                       0);
        if (n <= 0) break;
        sent += static_cast<size_t>(n);
    }
}

// ── Constructor / Destructor ──────────────────────────────────

HttpServer::HttpServer(int port,
                       AuthManager&   auth,
                       WordValidator& validator,
                       RoomManager&   rooms)
    : port_(port), auth_(auth), validator_(validator), rooms_(rooms)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
}

HttpServer::~HttpServer() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// ── Main listen loop ──────────────────────────────────────────

void HttpServer::run() {
    sock_t server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == SOCK_INVALID) {
        std::cerr << "[Server] socket() failed\n"; return;
    }

    int opt = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (::bind(server_fd,
               reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) == SOCK_ERR) {
        std::cerr << "[Server] bind() failed on port " << port_ << "\n";
        sock_close(server_fd);
        return;
    }
    ::listen(server_fd, 128);
    std::cout << "[ThinkFast] C++ backend listening on port " << port_ << "\n";

    while (running_) {
        sock_t client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd == SOCK_INVALID) continue;
        std::thread([this, client_fd]() {
            handleClient(client_fd);
            sock_close(client_fd);
        }).detach();
    }
    sock_close(server_fd);
}

void HttpServer::stop() { running_ = false; }

// ── Per-connection handler ────────────────────────────────────

void HttpServer::handleClient(sock_t fd) {
    char buf[65536];
    int  total = 0;

    while (total < static_cast<int>(sizeof(buf)) - 1) {
        int n = ::recv(fd, buf + total,
                       static_cast<int>(sizeof(buf) - 1) - total, 0);
        if (n <= 0) break;
        total += n;
        buf[total] = '\0';

        std::string raw(buf, static_cast<size_t>(total));
        auto hdrEnd = raw.find("\r\n\r\n");
        if (hdrEnd != std::string::npos) {
            std::string lower = raw.substr(0, hdrEnd);
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            auto clPos = lower.find("content-length:");
            if (clPos == std::string::npos) break;
            size_t cl = static_cast<size_t>(std::stoi(lower.substr(clPos + 15)));
            if (raw.size() >= hdrEnd + 4 + cl) break;
        }
    }
    if (total <= 0) return;

    std::string raw(buf, static_cast<size_t>(total));

    auto lineEnd = raw.find("\r\n");
    if (lineEnd == std::string::npos) return;

    std::istringstream rl(raw.substr(0, lineEnd));
    std::string method, path, proto;
    rl >> method >> path >> proto;

    auto respond = [&](const std::string& body, int code = 200) {
        std::string status = (code == 200) ? "200 OK" : "400 Bad Request";
        std::string resp =
            "HTTP/1.1 " + status + "\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n\r\n" + body;
        sendAll(fd, resp);
    };

    if (method == "OPTIONS") { respond(""); return; }
    if (method != "POST")    { respond(err("Only POST allowed"), 400); return; }

    auto hdrEnd = raw.find("\r\n\r\n");
    std::string body = (hdrEnd != std::string::npos) ? raw.substr(hdrEnd + 4) : "";

    respond(dispatch(path, body));
}

// ── Route dispatcher ──────────────────────────────────────────

std::string HttpServer::dispatch(const std::string& path, const std::string& body) {
    if (path == "/auth"         || path == "/auth/")         return routeAuth(body);
    if (path == "/validate"     || path == "/validate/")     return routeValidate(body);
    if (path == "/leaderboard"  || path == "/leaderboard/")  return routeLeaderboard(body);
    if (path == "/room"         || path == "/room/")         return routeRoom(body);
    return err("Unknown endpoint: " + path);
}

// ── /auth ─────────────────────────────────────────────────────

std::string HttpServer::routeAuth(const std::string& body) {
    std::string action   = jsonGet(body, "action");
    std::string username = jsonGet(body, "username");
    std::string password = jsonGet(body, "password");

    std::lock_guard<std::mutex> lk(authMu);
    auth_.reload();

    if (action == "login") {
        Player* p = auth_.login(username, password);
        if (!p) return err("Invalid username or password.");
        return ok("\"username\":\""    + jsonEscape(p->username)    + "\","
                  "\"wins\":"          + std::to_string(p->wins)    + ","
                  "\"losses\":"        + std::to_string(p->losses)  + ","
                  "\"games_played\":"  + std::to_string(p->games)   + ","
                  "\"joined_date\":\"" + jsonEscape(p->joined_date) + "\","
                  "\"guest\":false");
    }
    if (action == "register") {
        if (username.size() < 2) return err("Username must be at least 2 characters.");
        if (password.size() < 3) return err("Password must be at least 3 characters.");
        if (auth_.userExists(username)) return err("Username already taken.");
        if (!auth_.registerUser(username, password)) return err("Registration failed.");
        Player* p = auth_.currentPlayer();
        return ok("\"username\":\""    + jsonEscape(p->username)    + "\","
                  "\"wins\":0,\"losses\":0,\"games_played\":0,"
                  "\"joined_date\":\"" + jsonEscape(p->joined_date) + "\","
                  "\"guest\":false");
    }
    if (action == "guest") {
        std::string name = jsonGet(body, "name");
        if (name.empty()) name = "Guest";
        return ok("\"username\":\"" + jsonEscape(name) + "\","
                  "\"wins\":0,\"losses\":0,\"games_played\":0,\"guest\":true");
    }
    if (action == "stats") {
        Player tmp;
        tmp.username = username;
        tmp.wins     = jsonGetInt(body, "wins");
        tmp.losses   = jsonGetInt(body, "losses");
        tmp.games    = jsonGetInt(body, "games");
        tmp.is_guest = false;
        auth_.saveStats(tmp);
        return ok("\"message\":\"Stats saved.\"");
    }
    return err("Unknown action: " + action);
}

// ── /validate ─────────────────────────────────────────────────

std::string HttpServer::routeValidate(const std::string& body) {
    std::string word = jsonGet(body, "word");
    if (word.empty()) return err("No word provided.");
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    bool valid = validator_.isValid(word);
    return ok("\"word\":\""  + jsonEscape(word) + "\","
              "\"valid\":"   + (valid ? "true" : "false"));
}

// ── /leaderboard ──────────────────────────────────────────────

std::string HttpServer::routeLeaderboard(const std::string& /*body*/) {
    std::lock_guard<std::mutex> lk(authMu);
    auth_.reload();
    auto rows = auth_.leaderboard();

    std::string arr = "[";
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        if (i) arr += ",";
        arr += "{\"username\":\""   + jsonEscape(r.username)         + "\","
               "\"wins\":"          + std::to_string(r.wins)         + ","
               "\"losses\":"        + std::to_string(r.losses)       + ","
               "\"games_played\":"  + std::to_string(r.games_played) + ","
               "\"joined_date\":\"" + jsonEscape(r.joined_date)      + "\"}";
    }
    arr += "]";
    return ok("\"players\":" + arr);
}

// ── /room ─────────────────────────────────────────────────────

std::string HttpServer::routeRoom(const std::string& body) {
    std::string action = jsonGet(body, "action");
    std::string player = jsonGet(body, "player");
    std::string code   = jsonGet(body, "code");
    std::transform(code.begin(), code.end(), code.begin(), ::toupper);

    if (action == "create") {
        std::string mode = jsonGet(body, "mode");
        if (mode.empty()) mode = "last_letter";
        return rooms_.create(player, mode,
                             jsonGetInt(body, "time_limit",  15),
                             jsonGetInt(body, "max_players", 4));
    }
    if (action == "join")  return rooms_.join (player, code);
    if (action == "state") return rooms_.state(player, code);
    if (action == "start") return rooms_.start(player, code);
    if (action == "move")  return rooms_.move (player, code, jsonGet(body, "value"));
    if (action == "leave") return rooms_.leave(player, code);
    return err("Unknown room action: " + action);
}

// ── JSON helpers ──────────────────────────────────────────────

std::string HttpServer::jsonGet(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos += needle.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':')) ++pos;
    if (pos >= json.size()) return "";

    if (json[pos] == '"') {
        ++pos;
        std::string val;
        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                ++pos;
                switch (json[pos]) {
                    case '"':  val += '"';  break;
                    case '\\': val += '\\'; break;
                    case 'n':  val += '\n'; break;
                    case 'r':  val += '\r'; break;
                    case 't':  val += '\t'; break;
                    default:   val += json[pos]; break;
                }
            } else {
                val += json[pos];
            }
            ++pos;
        }
        return val;
    } else {
        std::string val;
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}'
               && json[pos] != ' ' && json[pos] != '\n')
            val += json[pos++];
        return val;
    }
}

int HttpServer::jsonGetInt(const std::string& json, const std::string& key, int def) {
    std::string v = jsonGet(json, key);
    if (v.empty()) return def;
    try { return std::stoi(v); } catch (...) { return def; }
}

std::string HttpServer::jsonEscape(const std::string& s) {
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

std::string HttpServer::ok(const std::string& body) {
    if (body.empty()) return "{\"ok\":true}";
    return "{\"ok\":true," + body + "}";
}

std::string HttpServer::err(const std::string& msg) {
    return "{\"ok\":false,\"error\":\"" + jsonEscape(msg) + "\"}";
}

} // namespace ThinkFast
