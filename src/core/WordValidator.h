#pragma once
// ============================================================
//  ThinkFast  |  src/core/WordValidator.h
//
//  Two-layer word validation:
//    1. Fast local lookup in a built-in dictionary (instant)
//    2. Live HTTP call to Free Dictionary API for words not
//       in the local set (requires network on user machine)
//
//  API used: https://api.dictionaryapi.dev/api/v2/entries/en/{word}
//  Returns 200 if the word is real, 404 if not.
//  Curl binary is invoked via popen() — no libcurl headers needed.
// ============================================================

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <random>

namespace ThinkFast {

class WordValidator {
public:
    WordValidator();

    // Optionally extend the local dictionary from a CSV file (one word per line)
    void loadFromCSV(const std::string& filepath);

    // Is the word valid English?
    // Checks local dict first; falls back to API for unknown words.
    bool isValid(const std::string& word);

    // Check without API (instant, local only)
    bool isValidLocal(const std::string& word) const;

    // Does any dictionary word start with this prefix? (local only)
    bool isValidPrefix(const std::string& prefix) const;

    // Last character of word (lowercased)
    char lastChar(const std::string& word) const;

    // Does the word start with the given character?
    bool startsWithChar(const std::string& word, char ch) const;

    // CPU: pick a valid word starting with startLetter, not in usedWords
    std::string cpuPickWord(char startLetter,
                            const std::unordered_set<std::string>& usedWords,
                            int difficulty = 1) const;

    // CPU: pick next letter for One-by-One mode
    char cpuNextLetter(const std::string& partial) const;

    // Random word to start a game (4-6 letters)
    std::string randomStartWord() const;

    // How many words in the local dictionary
    size_t dictionarySize() const;

private:
    std::unordered_set<std::string>          dict_;
    std::vector<std::string>                 wordList_;
    mutable std::unordered_map<std::string,bool> apiCache_; // word -> valid
    mutable std::mt19937                     rng_;

    void buildBuiltinDictionary();

    // Calls the Free Dictionary API via curl subprocess
    bool queryAPI(const std::string& word) const;

    static std::string toLower(const std::string& s);
    static std::string trim(const std::string& s);
    static std::string stripQuotes(const std::string& s);
};

} // namespace ThinkFast
