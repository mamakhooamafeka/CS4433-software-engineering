#include "presentation/ConsoleIO.hpp"

#include <cmath>
#include <cstdio>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lms {
namespace {

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

// Turns terminal echo off for the lifetime of the object (RAII), only for a real console.
class EchoOff {
public:
    explicit EchoOff(bool active) : active_(active) {
        if (!active_) return;
#ifdef _WIN32
        handle_ = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(handle_, &mode_);
        SetConsoleMode(handle_, mode_ & ~static_cast<DWORD>(ENABLE_ECHO_INPUT));
#else
        tcgetattr(STDIN_FILENO, &old_);
        termios quiet = old_;
        quiet.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &quiet);
#endif
    }
    ~EchoOff() {
        if (!active_) return;
#ifdef _WIN32
        SetConsoleMode(handle_, mode_);
#else
        tcsetattr(STDIN_FILENO, TCSANOW, &old_);
#endif
    }
    EchoOff(const EchoOff&) = delete;
    EchoOff& operator=(const EchoOff&) = delete;

private:
    bool active_;
#ifdef _WIN32
    HANDLE handle_ = nullptr;
    DWORD mode_ = 0;
#else
    termios old_{};
#endif
};

bool isRealConsole(const std::istream& in) {
#ifdef _WIN32
    DWORD mode = 0;
    return &in == &std::cin && GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
#else
    return &in == &std::cin && isatty(STDIN_FILENO) != 0;
#endif
}

} // namespace

std::string ConsoleIO::readLine(const std::string& prompt) {
    out_ << prompt << std::flush;
    std::string text;
    if (!std::getline(in_, text)) {
        throw InputClosed();
    }
    return text;
}

int ConsoleIO::readInt(const std::string& prompt, int min, int max) {
    while (true) {
        const std::string text = trim(readLine(prompt));
        try {
            std::size_t used = 0;
            const long long value = std::stoll(text, &used);
            if (used == text.size() && value >= min && value <= max) {
                return static_cast<int>(value);
            }
        } catch (const std::exception&) {
            // not a number: fall through to the message
        }
        out_ << "  Please enter a whole number from " << min << " to " << max << ".\n";
    }
}

double ConsoleIO::readMark(const std::string& prompt, int max) {
    while (true) {
        const std::string text = trim(readLine(prompt));
        try {
            std::size_t used = 0;
            const double value = std::stod(text, &used);
            const bool oneDecimal = std::fabs(value * 10 - std::round(value * 10)) < 1e-9;
            if (used == text.size() && std::isfinite(value) && value >= 0 && value <= max &&
                oneDecimal) {
                return value;
            }
        } catch (const std::exception&) {
            // not a number
        }
        out_ << "  Please enter a mark from 0 to " << max << ", with at most one decimal.\n";
    }
}

std::string ConsoleIO::readText(const std::string& prompt, std::size_t maxLen, bool allowEmpty) {
    while (true) {
        const std::string text = trim(readLine(prompt));
        if (text.empty() && !allowEmpty) {
            out_ << "  This cannot be empty.\n";
        } else if (text.size() > maxLen) {
            out_ << "  Please use at most " << maxLen << " characters.\n";
        } else {
            return text;
        }
    }
}

std::string ConsoleIO::readPassword(const std::string& prompt) {
    std::string text;
    {
        EchoOff guard(isRealConsole(in_));
        text = readLine(prompt);
    }
    if (isRealConsole(in_)) out_ << '\n';
    if (!text.empty() && text.back() == '\r') text.pop_back();
    return text;
}

DateTime ConsoleIO::readDateTime(const std::string& prompt) {
    while (true) {
        try {
            return fromIso8601(trim(readLine(prompt)));
        } catch (const ValidationError& e) {
            out_ << "  " << e.what() << '\n';
        }
    }
}

bool ConsoleIO::confirm(const std::string& prompt) {
    while (true) {
        const std::string text = trim(readLine(prompt + " (y/n): "));
        if (text == "y" || text == "Y" || text == "yes") return true;
        if (text == "n" || text == "N" || text == "no") return false;
        out_ << "  Please type y or n.\n";
    }
}

int ConsoleIO::choose(const std::string& title, const std::vector<std::string>& items) {
    heading(title);
    if (items.empty()) {
        out_ << "  There is nothing to show.\n";
        return -1;
    }
    for (std::size_t i = 0; i < items.size(); ++i) {
        out_ << "  " << (i + 1) << ". " << fit(items[i], kWidth - 8) << '\n';
    }
    out_ << "  0. Back\n";
    return readInt("Choose: ", 0, static_cast<int>(items.size())) - 1;
}

void ConsoleIO::showError(const LmsError& e) {
    out_ << "\n  ! " << e.what() << "\n";
}

void ConsoleIO::heading(const std::string& title) {
    out_ << "\n" << fit(title, kWidth) << "\n" << std::string(std::min(title.size(), kWidth), '-')
         << "\n";
}

void ConsoleIO::line(const std::string& text) {
    out_ << fit(text, kWidth) << '\n';
}

void ConsoleIO::info(const std::string& text) {
    out_ << "  " << fit(text, kWidth - 2) << '\n';
}

std::string showTime(DateTime t) {
    const std::string iso = toIso8601(t);   // 2026-11-07T23:59:00Z
    return iso.substr(0, 10) + " " + iso.substr(11, 5);
}

std::string showMark(double mark) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.1f", mark);
    return buf;
}

std::string fit(const std::string& text, std::size_t width) {
    if (text.size() <= width) return text;
    if (width <= 3) return text.substr(0, width);
    return text.substr(0, width - 3) + "...";
}

} // namespace lms
