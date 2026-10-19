#include "data/SqliteDatabase.hpp"

#include "SchemaSql.hpp"
#include "infrastructure/ErrorLog.hpp"

#include <sqlite3.h>

namespace lms {

void throwSqliteError(sqlite3* db, int rc, const std::string& context) {
    const std::string detail =
        context + ": " + (db ? sqlite3_errmsg(db) : sqlite3_errstr(rc)) + " (code " +
        std::to_string(rc) + ")";
    ErrorLog::write(detail);
    if ((rc & 0xff) == SQLITE_CONSTRAINT) {
        throw ConstraintViolation("The data breaks a rule of the system.");
    }
    if ((rc & 0xff) == SQLITE_BUSY || (rc & 0xff) == SQLITE_LOCKED) {
        throw DataAccessError("The database is busy. Please try again.");
    }
    throw DataAccessError("A database error occurred, so the change was not saved.");
}

// ---- Statement ---------------------------------------------------------------

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
    const int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr);
    if (rc != SQLITE_OK) {
        throwSqliteError(db_, rc, "prepare");
    }
}

Statement::~Statement() {
    sqlite3_finalize(stmt_);
}

Statement::Statement(Statement&& other) noexcept : db_(other.db_), stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

namespace {
void check(sqlite3* db, int rc) {
    if (rc != SQLITE_OK) throwSqliteError(db, rc, "bind");
}
} // namespace

Statement& Statement::bind(int index, int value) {
    check(db_, sqlite3_bind_int(stmt_, index, value));
    return *this;
}

Statement& Statement::bind(int index, std::int64_t value) {
    check(db_, sqlite3_bind_int64(stmt_, index, static_cast<sqlite3_int64>(value)));
    return *this;
}

Statement& Statement::bind(int index, double value) {
    check(db_, sqlite3_bind_double(stmt_, index, value));
    return *this;
}

Statement& Statement::bind(int index, bool value) {
    return bind(index, value ? 1 : 0);
}

Statement& Statement::bind(int index, const std::string& value) {
    check(db_, sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()),
                                 SQLITE_TRANSIENT));
    return *this;
}

Statement& Statement::bind(int index, DateTime value) {
    return bind(index, toIso8601(value));
}

Statement& Statement::bindNull(int index) {
    check(db_, sqlite3_bind_null(stmt_, index));
    return *this;
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throwSqliteError(db_, sqlite3_extended_errcode(db_), "step");
}

void Statement::run() {
    while (step()) {
    }
}

bool Statement::isNull(int col) const {
    return sqlite3_column_type(stmt_, col) == SQLITE_NULL;
}

int Statement::getInt(int col) const {
    return sqlite3_column_int(stmt_, col);
}

std::int64_t Statement::getInt64(int col) const {
    return static_cast<std::int64_t>(sqlite3_column_int64(stmt_, col));
}

double Statement::getDouble(int col) const {
    return sqlite3_column_double(stmt_, col);
}

bool Statement::getBool(int col) const {
    return getInt(col) != 0;
}

std::string Statement::getText(int col) const {
    const auto* text = sqlite3_column_text(stmt_, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

DateTime Statement::getDateTime(int col) const {
    return fromIso8601(getText(col));
}

std::optional<int> Statement::getOptionalInt(int col) const {
    if (isNull(col)) return std::nullopt;
    return getInt(col);
}

std::optional<double> Statement::getOptionalDouble(int col) const {
    if (isNull(col)) return std::nullopt;
    return getDouble(col);
}

std::optional<DateTime> Statement::getOptionalDateTime(int col) const {
    if (isNull(col)) return std::nullopt;
    return getDateTime(col);
}

// ---- SqliteDatabase ----------------------------------------------------------

SqliteDatabase::SqliteDatabase(const std::string& path) {
    const int rc = sqlite3_open(path.c_str(), &handle_);
    if (rc != SQLITE_OK) {
        sqlite3* failed = handle_;
        handle_ = nullptr;
        ErrorLog::write("open " + path + ": " + sqlite3_errmsg(failed));
        sqlite3_close(failed);
        throw DataAccessError("The database could not be opened.");
    }
    sqlite3_busy_timeout(handle_, 2000);
    exec("pragma foreign_keys = on;");
    exec("pragma synchronous = full;");   // NFR-05: survive a power loss during a write
    createSchemaIfNew();
}

SqliteDatabase::~SqliteDatabase() {
    sqlite3_close(handle_);
}

void SqliteDatabase::exec(const std::string& sql) {
    char* message = nullptr;
    const int rc = sqlite3_exec(handle_, sql.c_str(), nullptr, nullptr, &message);
    sqlite3_free(message);
    if (rc != SQLITE_OK) {
        throwSqliteError(handle_, rc, "exec");
    }
}

void SqliteDatabase::createSchemaIfNew() {
    Statement s = prepare("select count(*) from sqlite_master where type = 'table' and name = ?");
    s.bind(1, std::string("user_account"));
    s.step();
    if (s.getInt(0) == 0) {
        exec(data::kSchemaSql);
    }
}

Statement SqliteDatabase::prepare(const std::string& sql) {
    return Statement(handle_, sql);
}

std::int64_t SqliteDatabase::lastInsertId() const {
    return static_cast<std::int64_t>(sqlite3_last_insert_rowid(handle_));
}

bool SqliteDatabase::integrityCheck() {
    Statement s = prepare("pragma integrity_check");
    return s.step() && s.getText(0) == "ok";
}

void SqliteDatabase::backupTo(const std::string& path) {
    sqlite3* dest = nullptr;
    if (sqlite3_open(path.c_str(), &dest) != SQLITE_OK) {
        sqlite3_close(dest);
        throw DataAccessError("The backup file could not be created.");
    }
    sqlite3_backup* backup = sqlite3_backup_init(dest, "main", handle_, "main");
    int rc = SQLITE_ERROR;
    if (backup) {
        rc = sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    sqlite3_close(dest);
    if (rc != SQLITE_DONE) {
        throwSqliteError(nullptr, rc, "backup");
    }
}

void SqliteDatabase::begin() {
    exec("begin immediate;");
}

void SqliteDatabase::commit() {
    exec("commit;");
}

void SqliteDatabase::rollback() {
    exec("rollback;");
}

} // namespace lms
