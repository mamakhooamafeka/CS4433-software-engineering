pragma foreign_keys = on;

create table user_account (
    id                   integer primary key,
    username             text    not null unique collate nocase
                                 check (length(username) between 3 and 32),
    full_name            text    not null check (length(full_name) between 1 and 100),
    password_hash        text    not null,
    role                 text    not null check (role in ('STUDENT','LECTURER','HOD','ADMIN')),
    active               integer not null default 1 check (active in (0,1)),
    failed_attempts      integer not null default 0 check (failed_attempts >= 0),
    locked_until         text,
    must_change_password integer not null default 1 check (must_change_password in (0,1))
);

create table department (
    id          integer primary key,
    name        text    not null unique,
    hod_user_id integer unique references user_account(id)
);

create table course (
    id            integer primary key,
    code          text    not null unique,
    title         text    not null,
    department_id integer not null references department(id)
);

create table course_lecturer (
    course_id   integer not null references course(id),
    lecturer_id integer not null references user_account(id),
    primary key (course_id, lecturer_id)
);

create table enrolment (
    course_id   integer not null references course(id),
    student_id  integer not null references user_account(id),
    enrolled_at text    not null,
    withdrawn   integer not null default 0 check (withdrawn in (0,1)),
    primary key (course_id, student_id)
);

create table assignment (
    id           integer primary key,
    course_id    integer not null references course(id),
    title        text    not null check (length(title) between 1 and 200),
    instructions text    not null,
    deadline     text    not null,
    max_mark     integer not null check (max_mark between 1 and 1000),
    created_by   integer not null references user_account(id),
    created_at   text    not null
);

create table extension (
    id            integer primary key,
    assignment_id integer not null references assignment(id),
    student_id    integer not null references user_account(id),
    new_deadline  text    not null,
    reason        text    not null check (length(trim(reason)) > 0),
    granted_by    integer not null references user_account(id),
    granted_at    text    not null
);

create table submission (
    id            integer primary key,
    assignment_id integer not null references assignment(id),
    student_id    integer not null references user_account(id),
    status        text    not null
                  check (status in ('SUBMITTED','LATE','MARKED','MODERATED','RELEASED')),
    raw_mark      real    check (raw_mark >= 0),
    deduction     real    check (deduction >= 0),
    final_mark    real    check (final_mark >= 0),
    days_late     integer check (days_late >= 0),
    feedback      text    check (feedback is null or length(feedback) <= 2000),
    marked_by     integer references user_account(id),
    marked_at     text,
    moderated_by  integer references user_account(id),
    moderated_at  text,
    released_by   integer references user_account(id),
    released_at   text,
    unique (assignment_id, student_id),
    check ((status in ('SUBMITTED','LATE') and raw_mark is null and final_mark is null)
        or (status in ('MARKED','MODERATED','RELEASED')
            and raw_mark is not null and deduction is not null and final_mark is not null
            and days_late is not null and length(trim(feedback)) > 0
            and marked_by is not null and marked_at is not null)),
    check (final_mark is null or abs(final_mark - (raw_mark - deduction)) < 0.05),
    check (status not in ('MODERATED','RELEASED') or moderated_by is not null),
    check (status <> 'RELEASED' or (released_by is not null and released_at is not null))
);

create table submission_version (
    id            integer primary key,
    submission_id integer not null references submission(id),
    version_no    integer not null check (version_no >= 1),
    stored_path   text    not null unique,
    original_name text    not null,
    size_bytes    integer not null check (size_bytes between 1 and 10485760),
    sha256        text    not null check (length(sha256) = 64),
    submitted_at  text    not null,
    unique (submission_id, version_no)
);

create table mark_audit (
    id             integer primary key,
    submission_id  integer not null references submission(id),
    changed_by     integer not null references user_account(id),
    changed_at     text    not null,
    change_type    text    not null check (change_type in ('INITIAL','CORRECTION','AMENDMENT')),
    old_raw_mark   real,
    old_final_mark real,
    new_raw_mark   real    not null,
    new_final_mark real    not null,
    reason         text    not null check (length(trim(reason)) > 0),
    check ((change_type = 'INITIAL') = (old_raw_mark is null))
);

create trigger mark_audit_no_update before update on mark_audit
begin select raise(abort, 'mark_audit is append-only (BR-05)'); end;
create trigger mark_audit_no_delete before delete on mark_audit
begin select raise(abort, 'mark_audit is append-only (BR-05)'); end;

create trigger submission_version_no_update before update on submission_version
begin select raise(abort, 'submission versions are read-only (BR-03)'); end;

create index idx_course_department     on course(department_id);
create index idx_course_lecturer_lect  on course_lecturer(lecturer_id);
create index idx_enrolment_student     on enrolment(student_id);
create index idx_assignment_course     on assignment(course_id);
create index idx_extension_lookup      on extension(assignment_id, student_id);
create index idx_submission_status     on submission(assignment_id, status);
create index idx_submission_student    on submission(student_id);
create index idx_audit_submission      on mark_audit(submission_id);
