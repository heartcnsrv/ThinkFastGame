#include "CSVManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace ThinkFast {

std::vector<CSVManager::UserRecord>
CSVManager::loadUsers(const std::string& filepath) {
    std::vector<UserRecord> users;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[CSVManager] Cannot open: " << filepath << "\n";
        return users;
    }
    std::string line;
    bool first = true;
    while (std::getline(file, line)) {
        if (first) { first = false; continue; }
        // Turn a CSV row into a typed record that higher layers can use.
        auto f = parseLine(line);
        if (f.size() < 6) continue;
        UserRecord u;
        u.username     = stripQuotes(f[0]);
        u.password     = stripQuotes(f[1]);
        u.wins         = safeInt(f[2]);
        u.losses       = safeInt(f[3]);
        u.games_played = safeInt(f[4]);
        u.joined_date  = stripQuotes(f[5]);
        if (!u.username.empty()) users.push_back(u);
    }
    return users;
}

bool CSVManager::saveUsers(const std::string& filepath,
                           const std::vector<UserRecord>& users) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[CSVManager] Cannot write: " << filepath << "\n";
        return false;
    }
    file << "\"username\",\"password\",\"wins\",\"losses\","
            "\"games_played\",\"joined_date\"\n";
    // The whole logical table is rewritten each time for simplicity.
    for (const auto& u : users) {
        file << '"' << escape(u.username)    << "\","
             << '"' << escape(u.password)    << "\","
             << '"' << u.wins                << "\","
             << '"' << u.losses              << "\","
             << '"' << u.games_played        << "\","
             << '"' << escape(u.joined_date) << "\"\n";
    }
    return true;
}

std::vector<std::string>
CSVManager::loadWords(const std::string& filepath) {
    std::vector<std::string> words;
    std::ifstream file(filepath);
    if (!file.is_open()) return words;
    std::string line;
    bool first = true;
    while (std::getline(file, line)) {
        if (first) { first = false; continue; }
        std::string w = stripQuotes(trim(line));
        if (w.empty()) continue;
        bool ok = true;
        for (unsigned char c : w)
            if (!std::isalpha(c)) { ok = false; break; }
        if (!ok || w.size() < 2) continue;
        std::transform(w.begin(), w.end(), w.begin(), ::tolower);
        words.push_back(w);
    }
    return words;
}


std::vector<std::string> CSVManager::parseLine(const std::string& line) {
    // Minimal CSV parser that keeps commas inside quoted fields intact.
    std::vector<std::string> fields;
    bool inQ = false;
    std::string cur;
    for (char c : line) {
        if (c == '"')                 { inQ = !inQ; cur += c; }
        else if (c == ',' && !inQ)   { fields.push_back(cur); cur.clear(); }
        else                          cur += c;
    }
    fields.push_back(cur);
    return fields;
}

std::string CSVManager::stripQuotes(const std::string& s) {
    const std::string t = trim(s);
    if (t.size() >= 2 && t.front() == '"' && t.back() == '"')
        return t.substr(1, t.size() - 2);
    return t;
}

std::string CSVManager::trim(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    const size_t b = s.find_last_not_of(" \t\r\n");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

std::string CSVManager::escape(const std::string& s) {
    std::string r;
    for (char c : s) { if (c == '"') r += '"'; r += c; }
    return r;
}

int CSVManager::safeInt(const std::string& s) {
    try { return std::stoi(stripQuotes(s)); }
    catch (...) { return 0; }
}

}
