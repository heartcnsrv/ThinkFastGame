#pragma once
#include <string>
#include <vector>

namespace ThinkFast {

class CSVManager {
public:
    struct UserRecord {
        std::string username;
        std::string password;
        int         wins         = 0;
        int         losses       = 0;
        int         games_played = 0;
        std::string joined_date;
    };

    static std::vector<UserRecord> loadUsers(const std::string& filepath);
    static bool                    saveUsers(const std::string& filepath,
                                             const std::vector<UserRecord>& users);
    static std::vector<std::string> loadWords(const std::string& filepath);

private:
    static std::vector<std::string> parseLine(const std::string& line);
    static std::string stripQuotes(const std::string& s);
    static std::string trim(const std::string& s);
    static std::string escape(const std::string& s);
    static int         safeInt(const std::string& s);
};

}
