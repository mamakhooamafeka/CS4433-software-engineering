// Writes technical error detail to lms-error.log, so the user only ever sees the
// plain-language message (NFR-07) while the technician can still find the cause.
#pragma once

#include <string>

namespace lms {

class ErrorLog {
public:
    static void setPath(const std::string& path);
    static void write(const std::string& detail);
};

}  // namespace lms
