#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ThinkFast {

/*
 * Word validation service.
 * It checks the built-in dictionary first,
 * then falls back to a public API for unknown words
 * and caches the result for later reuse.
 */
class WordValidator {
public:
    WordValidator();

    /* Loads extra words from a CSV file into the local dictionary. */
    void loadFromCSV(const std::string& filepath);

    /* Returns true if the word is valid locally or through the API. */
    bool isValid(const std::string& word);

    /* Returns true only when the word exists in the local dictionary. */
    bool isValidLocal(const std::string& word) const;

    /* Checks whether any known word can continue from the prefix. */
    bool isValidPrefix(const std::string& prefix) const;

    /* Returns the last character of a word after normalization. */
    char lastChar(const std::string& word) const;

    /* Checks whether a word begins with a specific character. */
    bool startsWithChar(const std::string& word, char ch) const;

    /* Chooses a bot word that matches the required starting letter. */
    std::string cpuPickWord(char startLetter,
                            const std::unordered_set<std::string>& usedWords,
                            int difficulty = 1) const;

    /* Chooses one next letter for bot play in One-by-One mode. */
    char cpuNextLetter(const std::string& partial) const;

    /* Returns a random starting word for new rounds. */
    std::string randomStartWord() const;

    /* Returns the current number of locally stored words. */
    size_t dictionarySize() const;

private:
    std::unordered_set<std::string>          dict_;      /* Fast membership lookup. */
    std::vector<std::string>                 wordList_;  /* Ordered source for random picks. */
    mutable std::unordered_map<std::string, bool> apiCache_; /* Remote validation cache. */
    mutable std::mt19937                     rng_;       /* Random source for bot choices. */

    /* Builds the built-in starter dictionary compiled into the program. */
    void buildBuiltinDictionary();

    /* Queries the public dictionary API for unknown words. */
    bool queryAPI(const std::string& word) const;

    /* Normalizes text to lowercase. */
    static std::string toLower(const std::string& s);
    /* Removes surrounding whitespace. */
    static std::string trim(const std::string& s);
    /* Removes wrapping double quotes. */
    static std::string stripQuotes(const std::string& s);
};

}
