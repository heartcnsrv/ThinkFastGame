#pragma once
#include <string>
#include <vector>

namespace ThinkFast {

namespace Color {
    extern const char* RESET;
    extern const char* BOLD;
    extern const char* DIM;
    extern const char* WHITE;
    extern const char* GRAY;
    extern const char* DARK_GRAY;
    extern const char* BLUE;
    extern const char* CYAN;
    extern const char* GREEN;
    extern const char* YELLOW;
    extern const char* RED;
    extern const char* TEAL;
}

class ConsoleUI {
public:
    static constexpr int W = 64;

    static void clear();
    static void hideCursor();
    static void showCursor();

    static void hr(const char* col = Color::DARK_GRAY, int w = W);
    static void dhr(const char* col = Color::BLUE, int w = W);

    static std::string pad(const std::string& text, int width,
                           char fill = ' ', bool right = false);
    static std::string center(const std::string& text, int width = W);

    static void println(const std::string& s = "");
    static void info(const std::string& s);
    static void success(const std::string& s);
    static void error(const std::string& s);
    static void warn(const std::string& s);

    static void box(const std::vector<std::string>& lines,
                    int width      = W,
                    const char* border = Color::BLUE,
                    const char* text   = Color::WHITE);

    static void logo();

    static void pause(int ms = 1000);

    static size_t visLen(const std::string& s);

    static std::string prompt(const std::string& label,
                               const char* col = Color::CYAN);

    static int menu(const std::string& title,
                    const std::vector<std::string>& options,
                    const char* labelCol = Color::BLUE);
};

}
