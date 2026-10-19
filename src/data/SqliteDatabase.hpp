// The only owner of the SQLite connection (class diagram 2b). Statement wraps a
// prepared statement: values are always bound to ? parameters, never pasted into
// the SQL text, which prevents SQL injection (NFR-09).
#pragma once

#include "domain/DateTime.hpp"
#include "domain/Errors.hpp"
#include "domain/Repositories.hpp"

#include <cstdint>
#include <optional>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace lms {

// A UNIQUE, CHECK or FOREIGN KEY constraint was broken. Repositories turn this
// into a ValidationError or BusinessRuleError with a message that fits the action.
class ConstraintViolation : public DataAccessError {
public:
    using DataAccessError::DataAccessError;
};

class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&&) = delete;

    // Parameters are numbered from 1, as in SQLite. Each returns *this so calls can be chained.
    Statement& bind(int index, int value);
    Statement& bind(int index, std::int64_t value);
    Statement& bind(int index, double value);
    Statement& bind(int index, bool value);
    Statement& bind(int index, const std::string& value);
    // Without this, a string literal would silently convert to bool.
    Statement& bind(int index, const char* value) { return bind(index, std::string(value)); }
    Statement& bind(int index, DateTime value);
    Statement& bindNull(int index);
    template <typename T>
    Statement& bind(int index, const std::optional<T>& value) {
        return value ? bind(index, *value) : bindNull(index);
    }

    // Runs one step. Returns true while there is a row to read.
    bool step();
    // For INSERT/UPDATE/DELETE: runs to completion.
    void run();

    // Columns are numbered from 0, as in SQLite.
    bool isNull(int col) const;
    int getInt(int col) const;
    std::int64_t getInt64(int col) const;
    double getDouble(int col) const;
    bool getBool(int col) const;
    std::string getText(int col) const;
    DateTime getDateTime(int col) const;
    std::optional<int> getOptionalInt(int col) const;
    std::optional<double> getOptionalDouble(int col) const;
    std::optional<DateTime> getOptionalDateTime(int col) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_ = nullptr;
};

class SqliteDatabase : public ITransactionManager {
public:
    // Opens (or creates) the database file, switches on foreign keys and creates
    // the schema on first use. ":memory:" gives a private in-memory database for tests.
    explicit SqliteDatabase(const std::string& path);
    ~SqliteDatabase() override;
    SqliteDatabase(const SqliteDatabase&) = delete;
    SqliteDatabase& operator=(const SqliteDatabase&) = delete;

    Statement prepare(const std::string& sql);
    std::int64_t lastInsertId() const;
    bool integrityCheck();
    // Copies the whole database to another file with SQLite's online backup API (FR-21).
    void backupTo(const std::string& path);
    // Replaces the open database with a backup copy (FR-21). The backup is checked first.
    void restoreFrom(const std::string& path);

    void begin() override;
    void commit() override;
    void rollback() override;

private:
    void exec(const std::string& sql);
    void createSchemaIfNew();

    sqlite3* handle_ = nullptr;
};

// Throws the right exception for an SQLite result code; detail goes to the error log.
[[noreturn]] void throwSqliteError(sqlite3* db, int rc, const std::string& context);

} // namespace lms
