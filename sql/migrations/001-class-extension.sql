-- Migration 1 (release v1.1.0, change request CR1): class-wide extensions.
-- Applied by the program to any database whose user_version is below 1, so a v1.0.0
-- database is upgraded in place the first time v1.1.0 opens it, and a new database
-- gets exactly the same tables as an upgraded one.

create table if not exists class_extension (
    id            integer primary key,
    assignment_id integer not null references assignment(id),
    new_deadline  text    not null,
    reason        text    not null check (length(trim(reason)) > 0),
    granted_by    integer not null references user_account(id),
    granted_at    text    not null
);

create index if not exists idx_class_extension_assignment on class_extension(assignment_id);

pragma user_version = 1;
