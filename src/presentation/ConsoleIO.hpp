// All keyboard input and screen output (class diagram 2b). Every read keeps asking
// until the input is valid, so wrong typing can never crash the program (NFR-07).
// Streams are injected so tests can drive the menus with scripted input.
#pragma once

#include "domain/DateTime.hpp"
#include "domain/Errors.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace lms {

// Thrown when the input stream ends (e.g. Ctrl+Z / Ctrl+D), so the program can exit cleanly.
class InputClosed : public std::exception {
public:
    const char* what() const noexcept override { return "input closed"; }
};

class ConsoleIO {
public:
    static constexpr std::size_t kWidth = 80;  // NFR-11: at most 80 columns

    explicit ConsoleIO(std::istream& in = std::cin, std::ostream& out = std::cout)
        : in_(in), out_(out) {}

    int readInt(const std::string& prompt, int min, int max);
    // A mark from 0 to max with at most one decimal place (BR-01).
    double readMark(const std::string& prompt, int max);
    std::string readText(const std::string& prompt, std::size_t maxLen, bool allowEmpty = false);
    // Does not echo the typed characters when the input is a real console.
    std::string readPassword(const std::string& prompt);
    DateTime readDateTime(const std::string& prompt);
    bool confirm(const std::string& prompt);
    // Shows numbered choices and returns the chosen index (0-based), or -1 for "Back".
    int choose(const std::string& title, const std::vector<std::string>& items);

    void showError(const LmsError& e);
    void heading(const std::string& title);
    void line(const std::string& text = "");
    void info(const std::string& text);

private:
    std::string readLine(const std::string& prompt);

    std::istream& in_;
    std::ostream& out_;
};

// Formats a time as "2026-11-07 23:59" for the screen (UTC).
std::string showTime(DateTime t);
// Formats a mark with one decimal place.
std::string showMark(double mark);
// Cuts text to a column width, adding "..." if it was longer.
std::string fit(const std::string& text, std::size_t width);

}  // namespace lms
