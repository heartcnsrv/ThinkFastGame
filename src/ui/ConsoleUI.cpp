// ============================================================
//  ThinkFast  |  src/ui/ConsoleUI.cpp
// ============================================================

#include "ConsoleUI.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

namespace ThinkFast {

// ── Color constants ───────────────────────────────────────────

namespace Color {
    const char* RESET     = "\033[0m";
    const char* BOLD      = "\033[1m";
    const char* DIM       = "\033[2m";
    const char* WHITE     = "\033[97m";
    const char* GRAY      = "\033[37m";
    const char* DARK_GRAY = "\033[90m";
    const char* BLUE      = "\033[94m";
    const char* CYAN      = "\033[96m";
    const char* GREEN     = "\033[92m";
    const char* YELLOW    = "\033[93m";
    const char* RED       = "\033[91m";
    const char* TEAL      = "\033[36m";
}

// ── Terminal control ──────────────────────────────────────────

void ConsoleUI::clear()      { std::cout << "\033[2J\033[H"; std::cout.flush(); }
void ConsoleUI::hideCursor() { std::cout << "\033[?25l"; }
void ConsoleUI::showCursor() { std::cout << "\033[?25h"; }

// ── Lines ─────────────────────────────────────────────────────

void ConsoleUI::hr(const char* col, int w) {
    std::cout << col << std::string(w, '-') << Color::RESET << "\n";
}

void ConsoleUI::dhr(const char* col, int w) {
    std::cout << col << std::string(w, '=') << Color::RESET << "\n";
}

// ── Text helpers ──────────────────────────────────────────────

std::string ConsoleUI::pad(const std::string& text, int width,
                           char fill, bool right) {
    const int vl  = static_cast<int>(visLen(text));
    const int rem = std::max(0, width - vl);
    if (right) return std::string(rem, fill) + text;
    return text + std::string(rem, fill);
}

std::string ConsoleUI::center(const std::string& text, int width) {
    const int vl  = static_cast<int>(visLen(text));
    const int p   = std::max(0, (width - vl) / 2);
    return std::string(p, ' ') + text;
}

// ── Printing ──────────────────────────────────────────────────

void ConsoleUI::println(const std::string& s) {
    std::cout << s << "\n";
}

void ConsoleUI::info(const std::string& s) {
    std::cout << Color::CYAN << "  " << s << Color::RESET << "\n";
}

void ConsoleUI::success(const std::string& s) {
    std::cout << Color::GREEN << "  " << s << Color::RESET << "\n";
}

void ConsoleUI::error(const std::string& s) {
    std::cout << Color::RED << "  [!] " << s << Color::RESET << "\n";
}

void ConsoleUI::warn(const std::string& s) {
    std::cout << Color::YELLOW << "  [~] " << s << Color::RESET << "\n";
}

// ── Box ───────────────────────────────────────────────────────

void ConsoleUI::box(const std::vector<std::string>& lines,
                    int width,
                    const char* border,
                    const char* text) {
    const int inner = width - 4;
    std::cout << border << "+" << std::string(width - 2, '-') << "+\n";
    for (const auto& line : lines) {
        const int vl  = static_cast<int>(visLen(line));
        const int rem = std::max(0, inner - vl);
        std::cout << border << "| " << text << line
                  << std::string(rem, ' ')
                  << border << " |\n";
    }
    std::cout << "+" << std::string(width - 2, '-') << "+" << Color::RESET << "\n";
}

// ── Logo ──────────────────────────────────────────────────────

void ConsoleUI::logo() {
    std::cout << "\n";
    std::cout << Color::BOLD << Color::BLUE;
    std::cout << "  +---------------------------------------------------------+\n";
    std::cout << "  |                                                         |\n";
    std::cout << "  |   THINK FAST  --  Last Letter Word Game                 |\n";
    std::cout << "  |                                                         |\n";
    std::cout << "  +---------------------------------------------------------+\n";
    std::cout << Color::RESET;
    std::cout << Color::DARK_GRAY;
    std::cout << "  Chain words by their last letter. Outlast every opponent.\n";
    std::cout << Color::RESET << "\n";
}

// ── Wait ──────────────────────────────────────────────────────

void ConsoleUI::pause(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ── ANSI-aware string length ──────────────────────────────────

size_t ConsoleUI::visLen(const std::string& s) {
    size_t len = 0;
    bool   esc = false;
    for (unsigned char c : s) {
        if (c == '\033')          { esc = true;  continue; }
        if (esc)                  { if (std::isalpha(c)) esc = false; continue; }
        if (c < 0x80) ++len;
    }
    return len;
}

// ── Prompts ───────────────────────────────────────────────────

std::string ConsoleUI::prompt(const std::string& label, const char* col) {
    std::cout << col << "  " << label << ": " << Color::WHITE;
    std::string input;
    std::getline(std::cin, input);
    std::cout << Color::RESET;
    return input;
}

int ConsoleUI::menu(const std::string& title,
                    const std::vector<std::string>& options,
                    const char* labelCol) {
    std::cout << "\n" << labelCol << Color::BOLD
              << "  " << title << Color::RESET << "\n";
    hr(Color::DARK_GRAY);
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << Color::CYAN << "  [" << (i + 1) << "] "
                  << Color::WHITE << options[i] << "\n";
    }
    std::cout << Color::RESET;
    hr(Color::DARK_GRAY);

    while (true) {
        const std::string in = prompt("Choice");
        try {
            const int choice = std::stoi(in);
            if (choice >= 1 && choice <= static_cast<int>(options.size()))
                return choice - 1;
        } catch (...) {}
        error("Enter a number from 1 to " + std::to_string(options.size()));
    }
}

} // namespace ThinkFast
