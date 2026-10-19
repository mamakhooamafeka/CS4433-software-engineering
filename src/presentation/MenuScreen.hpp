// Base class of the four role menus (class diagram 2b). run() is a Template Method:
// it draws the menu from user.menuOptions(), handles the options every role shares,
// and passes the rest to the subclass's handle(). Any LmsError thrown by a service
// is shown in plain language and the menu comes back (NFR-07).
#pragma once

#include "domain/User.hpp"
#include "presentation/ConsoleIO.hpp"
#include "services/AuthService.hpp"

#include <optional>

namespace lms {

class MenuScreen {
public:
    MenuScreen(ConsoleIO& io, AuthService& auth) : io_(io), auth_(auth) {}
    virtual ~MenuScreen() = default;
    MenuScreen(const MenuScreen&) = delete;
    MenuScreen& operator=(const MenuScreen&) = delete;

    // Shows the menu until the user chooses "Log out".
    void run(User& user);

protected:
    // Carries out one role-specific option.
    virtual void handle(User& user, MenuOption option) = 0;

    ConsoleIO& io_;
    AuthService& auth_;
};

// Asks for a new password twice and saves it (UC02). Returns false if the user
// gave up. Used by every menu and by the forced change after a temporary password.
bool askNewPassword(ConsoleIO& io, AuthService& auth, User& user,
                    const std::optional<std::string>& currentPassword = std::nullopt);

// Asks for a password twice until both match and it has 8+ characters (FR-03).
std::string readNewPassword(ConsoleIO& io, const std::string& what);

} // namespace lms
