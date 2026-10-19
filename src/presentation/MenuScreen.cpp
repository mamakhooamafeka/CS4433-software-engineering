#include "presentation/MenuScreen.hpp"

#include "domain/Enums.hpp"
#include "services/Validation.hpp"

namespace lms {

void MenuScreen::run(User& user) {
    while (true) {
        const std::vector<MenuOption> options = user.menuOptions();
        io_.heading("Main menu: " + user.getFullName() + " (" + toString(user.role()) + ")");
        const int pending = auth_.pendingWorkCount(user);  // FR-23
        if (pending > 0) {
            const char* what = user.role() == Role::STUDENT ? " assignment(s) to submit"
                               : user.role() == Role::HOD   ? " submission(s) to moderate"
                                                            : " submission(s) to mark";
            io_.info("You have " + std::to_string(pending) + what + ".");
        }
        for (std::size_t i = 0; i < options.size(); ++i) {
            io_.info(std::to_string(i + 1) + ". " + label(options[i]));
        }
        const int pick = io_.readInt("Choose: ", 1, static_cast<int>(options.size()));
        const MenuOption option = options[static_cast<std::size_t>(pick - 1)];
        if (option == MenuOption::LOG_OUT) return;
        try {
            if (option == MenuOption::CHANGE_PASSWORD) {
                askNewPassword(io_, auth_, user);
            } else {
                handle(user, option);
            }
        } catch (const LmsError& e) {
            io_.showError(e);
        }
    }
}

std::string readNewPassword(ConsoleIO& io, const std::string& what) {
    while (true) {
        const std::string first = io.readPassword(what + " (at least 8 characters): ");
        try {
            requirePasswordStrength(first);
        } catch (const ValidationError& e) {
            io.showError(e);
            continue;
        }
        if (io.readPassword("Type it again: ") == first) return first;
        io.info("The two passwords are not the same. Please try again.");
    }
}

bool askNewPassword(ConsoleIO& io, AuthService& auth, User& user,
                    const std::optional<std::string>& currentPassword) {
    io.heading("Change password");
    for (int attempt = 0; attempt < 3; ++attempt) {
        const std::string current =
            currentPassword ? *currentPassword : io.readPassword("Current password: ");
        const std::string next = readNewPassword(io, "New password");
        try {
            auth.changePassword(user, current, next);
            io.info("Your password has been changed.");
            return true;
        } catch (const LmsError& e) {
            io.showError(e);
        }
    }
    return false;
}

}  // namespace lms
