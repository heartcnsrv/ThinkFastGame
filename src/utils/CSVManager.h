#pragma once

#include <string>
#include <vector>

namespace ThinkFast {

/*
 * CSV-backed persistence helper.
 * This class acts as the file storage layer of the project.
 * It only parses and writes CSV data.
 * Higher-level rules stay inside AuthManager and other services.
 */
class CSVManager {
public:
    /*
     * In-memory version of one row from users.csv.
     * AuthManager converts this record into the richer Player model.
     */
    struct UserRecord {
        std::string username;         /* Unique login name. */
        std::string password;         /* Plain-text password used by this demo. */
        int         wins         = 0; /* Stored win count. */
        int         losses       = 0; /* Stored loss count. */
        int         games_played = 0; /* Stored total matches played. */
        std::string joined_date;      /* Registration date string. */
    };

    /*
     * Loads all user rows from a CSV file.
     * - filepath: source CSV path
     * Returns: vector of parsed user records
     */
    static std::vector<UserRecord> loadUsers(const std::string& filepath);

    /*
     * Saves all user rows back into the CSV file.
     * - filepath: destination CSV path
     * - users: records to write
     * Returns: true if the file was written successfully
     */
    static bool saveUsers(const std::string& filepath,
                          const std::vector<UserRecord>& users);

    /*
     * Loads plain word entries from a CSV file.
     * - filepath: source CSV path
     * Returns: cleaned lowercase words
     */
    static std::vector<std::string> loadWords(const std::string& filepath);

private:
    /* Splits one CSV line into fields while respecting quoted commas. */
    static std::vector<std::string> parseLine(const std::string& line);
    /* Removes wrapping double quotes after parsing. */
    static std::string stripQuotes(const std::string& s);
    /* Trims surrounding whitespace characters. */
    static std::string trim(const std::string& s);
    /* Escapes quotes before writing a field back to CSV. */
    static std::string escape(const std::string& s);
    /* Safely converts a CSV field into an integer. */
    static int safeInt(const std::string& s);
};

}
