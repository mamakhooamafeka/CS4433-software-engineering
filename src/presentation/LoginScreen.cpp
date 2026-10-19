#include "presentation/LoginScreen.hpp"

#include "presentation/MenuScreen.hpp"

namespace lms {

std::unique_ptr<User> LoginScreen::run() {
    while (true) {
        io_.heading("Moderated LMS: log in");
        io_.info("Leave the username empty and press Enter to exit.");
        const std::string username = io_.readText("Username: ", 32, true);
        if (username.empty()) return nullptr;
        const std::string password = io_.readPassword("Password: ");
        try {
            std::unique_ptr<User> user = auth_.login(username, password);
            if (user->mustChangePassword()) {
                io_.info("You are using a temporary password. Please choose your own now.");
                if (!askNewPassword(io_, auth_, *user, password)) continue;
            }
            io_.info("Welcome, " + user->getFullName() + ".");
            return user;
        } catch (const LmsError& e) {
            io_.showError(e);
        }
    }
}

} // namespace lms
