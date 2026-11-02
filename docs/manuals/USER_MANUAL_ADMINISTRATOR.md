# Administrator manual

Moderated LMS v1.0.0. This manual is for the **administrator**. It shows how to create accounts, set up departments and courses, enrol students, reset passwords, and make and restore backups. The technician installs the program first (`INSTALL.md`).

## 1. Getting started

### Start the program

Double-click **Moderated LMS** on the Desktop of the school office computer. A black window opens with the log-in screen:

```
Moderated LMS: log in
---------------------
  Leave the username empty and press Enter to exit.
Username:
```

### Log in

1. Type your username and press Enter.
2. Type your password and press Enter. The password stays hidden while you type. That is normal.
3. The first time you log in, you are using a temporary password from the administrator. The program asks you to choose your own password straight away. It must have at least 8 characters, and you type it twice:
   ```
     You are using a temporary password. Please choose your own now.

   Change password
   ---------------
   New password (at least 8 characters):
   Type it again:
     Your password has been changed.
   ```

If you type a wrong password 5 times, your account is locked for 15 minutes. Wait, or ask the administrator to reset your password. Never tell anyone your password, and never write it on the computer.

### How to use the menus

| To do this | Do this |
|---|---|
| Choose a menu option | Type its number and press Enter |
| Go back without doing anything | Type `0` (the `0. Back` line) and press Enter |
| Answer a question that ends in `(y/n)` | Type `y` for yes or `n` for no, and press Enter. Nothing is saved until you answer `y` |
| Type a date and time | Use the form `YYYY-MM-DD HH:MM`: year, month and day joined with dashes, then a space, then the time on the 24-hour clock. For example, 2 pm is `14:00` |

If you type something the program cannot use, it explains what is wrong and asks again. Messages that start with `!` are errors; nothing was changed.

### Times are in UTC

The program shows and expects all times in UTC, which is **2 hours behind Lesotho time**. To type a Lesotho time, subtract 2 hours: a deadline of 16:00 Lesotho time is typed as `14:00`. To read a time on the screen, add 2 hours.

### Finish

1. Choose **Log out** in your main menu. Always log out before you leave the computer, so nobody else can use your account.
2. At the `Username:` prompt, press Enter without typing anything to close the program.

In the screen examples in this manual, `YYYY-MM-DD` stands for a real date shown by the program.

## 2. The very first start

The first time the program starts there are no accounts, so it asks you to create the administrator account:

```
First start: create the administrator account
---------------------------------------------
Username: admin
Full name: System Admin
Password (at least 8 characters):
Type it again:
  Administrator created. Please log in.
```

Choose a strong password and keep it safe. The administrator account can create and reset every other account.

## 3. Your main menu

```
Main menu: System Admin (ADMIN)
-------------------------------
  1. Manage user accounts
  2. Reset a user's password
  3. Manage departments and courses
  4. Assign lecturers and enrol students
  5. Back up or restore
  6. Change my password
  7. Log out
Choose:
```

## 4. Setting up a new term: the order to follow

Some things need others to exist first, so do them in this order:

1. Create the accounts for the heads of department, lecturers and students (section 5).
2. Create each department and choose its head (section 6).
3. Create the courses in each department (section 6).
4. Assign a lecturer to each course and enrol the students (section 7).
5. Give each person their username and temporary password, privately (for example on a slip of paper handed to them). They must choose their own password the first time they log in.

## 5. Manage user accounts

Choose `1. Manage user accounts`. You then see:

```
  1. List all accounts
  2. Create an account
  3. Edit a full name
  4. Deactivate an account
  0. Back
```

### Create an account

1. Choose `2. Create an account`.
2. Choose the role: `1. Student`, `2. Lecturer`, `3. Head of department` or `4. Administrator`.
3. Type the username and the full name. To stop without creating the account, press Enter on an empty username or full name.
4. Type a temporary password (at least 8 characters) twice.
   ```
   Username (Enter to cancel): stu1
   Full name (Enter to cancel): Thabo Student
   Temporary password (at least 8 characters):
   Type it again:
     Account created. The user must change the password at first log in.
   ```

Usernames must be different for every person. Use a fixed pattern, for example the learner's school number, so they are easy to find.

### List all accounts

`1. List all accounts` shows every account as: user number, username, full name and role:

```
Accounts
--------
  1 admin | System Admin | ADMIN
  2 hod1 | Dr Head | HOD
  3 lec1 | Mr Lecturer | LECTURER
  4 stu1 | Thabo Student | STUDENT
```

The head of department needs the user number to look up one student in the audit log.

### Edit a full name

`3. Edit a full name`, choose the account, and type the corrected name.

### Deactivate an account

When a learner or teacher leaves: `4. Deactivate an account`, choose the account, and type `y`. The person can no longer log in, but their submissions and marks are kept. Accounts are never deleted, because the marks must stay traceable.

## 6. Departments and courses

Choose `3. Manage departments and courses`:

```
  1. List departments and courses
  2. Create a department
  3. Change a department's head
  4. Create a course
  0. Back
```

| To do this | Do this |
|---|---|
| Create a department | `2`, type the name (for example `Computer Science`), choose the head of department from the list. The HoD account must exist first |
| Change the head | `3`, choose the department, choose the new head |
| Create a course | `4`, choose its department, type the course code (for example `CS4433`) and the title |
| Check what exists | `1` lists all departments and courses |

## 7. Assign lecturers and enrol students

Choose `4. Assign lecturers and enrol students`:

```
  1. Assign a lecturer to a course
  2. Enrol a student
  3. Withdraw a student
  0. Back
```

1. Choose what to do, then the course, then the lecturer or student.
2. The program confirms: `Lecturer assigned.` or `Student enrolled.`

To enrol a whole class, repeat `2. Enrol a student` for each learner.

Withdrawing a student (option `3`, then `y`) removes them from the course. Their earlier submissions and marks are kept.

## 8. Reset a password

When someone forgets their password or is locked out:

1. Choose `2. Reset a user's password` and choose the account.
2. Type a new temporary password twice.
3. Give it to the person privately. They must choose their own password when they log in. A reset also unlocks a locked account.

Check that the person asking is really the account owner before you reset it.

## 9. Backups

### Automatic backups

The program makes a backup by itself the first time it starts each day. You see `Today's backup has been made.` above the log-in screen. Backups are kept in `C:\lms-data\backups`, one folder per backup. Every school day at 16:30 Windows also copies them to the external backup disk (set up by the technician). Keep that disk plugged in.

### Make a backup now

Before any big change (for example before releasing a whole term's marks, or before installing a new version): choose `5. Back up or restore`, then `1. Back up now`. The program says `Backup saved as ...` with the folder name.

### Restore a backup

Restoring replaces **all** current data with the copy from the backup. Anything entered after that backup was made is lost.

1. Ask everyone to stop using the system.
2. Choose `5. Back up or restore`, then `2. Restore a backup`.
3. The backups are listed newest first; the folder name starts with the date and time it was made. Choose one.
4. Read the warning, then type `y`.
5. Log out and log in again.

Use a restore if the program says `The database file is damaged. Please restore a backup.`, or if data was entered wrongly on a large scale.

### Old backups

Backups are never deleted by the program. At the end of each term, the technician can move old backup folders from `C:\lms-data\backups` to storage that the school keeps safely, following the school's rules on how long learner records are kept.

## 10. Looking after learners' data

The system holds personal data about learners: names, submitted work and marks. As the administrator:

- give each person their own account, and never share accounts;
- deactivate accounts of people who have left;
- keep the backup disk in a locked place, because it holds a full copy of the data;
- keep the school office computer in a supervised room, and lock Windows (Windows key + L) when you leave it.

## 11. Problems

| Problem | What to do |
|---|---|
| A person is missing from a list when I assign or enrol | The account has the wrong role, or it is deactivated. Check with `1. Manage user accounts`, `1. List all accounts` |
| `The database file is damaged. Please restore a backup.` | Restore the newest backup (section 9) |
| No `Today's backup has been made.` message | A backup was already made today, which is normal. If a message starting with `!` appears instead, the backup failed: tell the technician |
| Something else goes wrong | The screen shows a short message. The details are in `C:\lms-data\lms-error.log`. Give that file to the technician |
