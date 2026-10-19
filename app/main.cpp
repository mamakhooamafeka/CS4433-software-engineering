// Entry point. WP 4.1 skeleton: later work packages wire up the services and menus here.
#include "domain/DateTime.hpp"

#include <chrono>
#include <iostream>

int main() {
    std::cout << "Moderated LMS v1.0\n"
              << "Server time: " << lms::toIso8601(std::chrono::system_clock::now()) << "\n";
    return 0;
}
