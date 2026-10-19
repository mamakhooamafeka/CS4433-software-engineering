// The real clock. Tests inject a fixed clock instead (IClock).
#pragma once

#include "domain/Infrastructure.hpp"

namespace lms {

class SystemClock : public IClock {
public:
    DateTime now() const override { return std::chrono::system_clock::now(); }
};

} // namespace lms
