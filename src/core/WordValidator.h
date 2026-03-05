#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <random>

namespace ThinkFast {

class WordValidator {
public:
    WordValidator();

    void loadFromCSV(const std::string& filepath);

    bool isValid(const std::string& word);

    bool isValidLocal(const std::string& word) const;

    bool isValidPrefix(const std::string& prefix) const;

    char lastChar(const std::string& word) const;

    bool startsWithChar(const std::string& word, char ch) const;

    std::string cpuPickWord(char startLetter,
                            const std::unordered_set<std::string>& usedWords,
                            int difficulty = 1) const;

    char cpuNextLetter(const std::string& partial) const;

    std::string randomStartWord() const;

    size_t dictionarySize() const;

private:
    std::unordered_set<std::string>          dict_;
    std::vector<std::string>                 wordList_;
    mutable std::unordered_map<std::string,bool> apiCache_; 
    mutable std::mt19937                     rng_;

    void buildBuiltinDictionary();

    bool queryAPI(const std::string& word) const;

    static std::string toLower(const std::string& s);
    static std::string trim(const std::string& s);
    static std::string stripQuotes(const std::string& s);
};

} 