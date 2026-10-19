// UC01 Log in (class diagram 2b). A user with a temporary password must choose a
// new one before reaching the menu (FR-05).
#pragma once

#include "presentation/ConsoleIO.hpp"
#include "services/AuthService.hpp"

#include <memory>

namespace lms {

class LoginScreen {
public:
    LoginScreen(ConsoleIO& io, AuthService& auth) : io_(io), auth_(auth) {}

    // Returns the logged-in user, or nullptr when the username is left empty (exit).
    std::unique_ptr<User> run();

private:
    ConsoleIO& io_;
    AuthService& auth_;
};

}  // namespace lms
