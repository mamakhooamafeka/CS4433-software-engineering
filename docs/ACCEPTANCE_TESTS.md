# Acceptance tests (run by hand on Windows)

These tests check that v1.0 does what each stakeholder needs, using the real program the way a real user would. Do them in order, because each one uses what the one before created. Write **Pass** or **Fail** and the date in the Result line of each test. If something fails, add it to `DEFECT_LOG.md`.

## Before you start

Open **PowerShell** (normal, not admin) and type these lines one at a time, pressing Enter after each:

```
cd $HOME\Documents\lms
.\build\Release\lms.exe C:\lms-test
```

`C:\lms-test` is a new, empty data folder just for these tests, so your real data in `C:\lms-data` is not touched.

Times in the program are in UTC. Lesotho time is UTC+2, so **UTC = your clock minus 2 hours**.

Passwords are hidden while you type them. That is normal.

---

## AT-01 Administrator sets up the system (UC02–UC07)

Stakeholder need: the administrator can prepare a semester without help.

Steps:

1. The program says "First start: create the administrator account". Type username `admin`, full name `System Admin`, password `Admin2026!` twice.
2. Log in as `admin` with `Admin2026!`.
3. Choose `1` (Manage accounts), then `2` (Create an account), then `3` (Head of department). Username `hod1`, full name `Dr Head`, temporary password `TempPass1` twice.
4. Choose `1`, `2`, `2` (Lecturer). Username `lec1`, full name `Mr Lecturer`, temporary password `TempPass1` twice.
5. Choose `1`, `2`, `1` (Student). Username `stu1`, full name `Thabo Student`, temporary password `TempPass1` twice.
6. Choose `3` (Departments and courses), then `2` (Create a department). Name `Computer Science`, then choose `1` (Dr Head).
7. Choose `3`, then `4` (Create a course), choose `1` (Computer Science). Code `CS4433`, title `Software Engineering`.
8. Choose `4` (Lecturers and enrolment), `1` (Assign a lecturer), `1` (CS4433), `1` (Mr Lecturer).
9. Choose `4`, `2` (Enrol a student), `1` (CS4433), `1` (Thabo Student).
10. Choose `1`, `1` (List all accounts).
11. Choose `7` (Log out), then press Enter on an empty Username to close the program.
12. Start it again with `.\build\Release\lms.exe C:\lms-test`.

Expected: every step says it was created, assigned or enrolled; step 10 lists 4 accounts; at step 12 the program goes straight to the log-in screen (it does **not** ask for a new first administrator, so the data was kept).

Result: ______

## AT-02 First log in forces a new password, and lockout (UC01, FR-02, FR-05)

Stakeholder need: accounts are safe, and nobody keeps the password the administrator chose.

Steps:

1. Log in as `stu1` with `TempPass1`.
2. When asked for a new password, type `short` once.
3. Then type `Student2026` and the same again.
4. Choose `5` (Log out).
5. Log in as `stu1` with the wrong password `wrong123`, five times in a row.
6. Now try `stu1` with the right password `Student2026`.

Expected: step 1 says you are using a temporary password; step 2 says it needs at least 8 characters; step 3 says the password has been changed and shows the student menu; step 6 is refused because the account is locked for 15 minutes.

Result: ______

(Continue with the other tests while `stu1` is locked. Log in as `stu1` again after 15 minutes.)

## AT-03 Lecturer creates assignments (UC09)

Stakeholder need: the lecturer can publish work with a clear deadline.

Steps:

1. Log in as `lec1` with `TempPass1`, and change the password to `Lecturer2026` (type it twice).
2. Choose `1` (Create assignment), `1` (CS4433). Title `Essay 1`, instructions `Write 500 words`, deadline **tomorrow** in the form `YYYY-MM-DD HH:MM` (for example `2026-11-08 21:00`), maximum mark `1001`, then `100`. Answer `y`.
3. Choose `1`, `1` again. Title `Quick quiz`, instructions `Answer the questions`, deadline **3 minutes from now in UTC** (your clock minus 2 hours, plus 3 minutes), maximum mark `50`. Answer `y`.
4. Choose `6` (Log out).

Expected: `1001` is refused and asked again; both assignments are created.

Result: ______

## AT-04 Student submits and resubmits on time (UC16, UC17)

Stakeholder need: the student gets proof of an on-time submission and can replace it before the deadline.

Steps:

1. Make a small file: in File Explorer, right-click on the Desktop, choose New > Text Document, name it `essay.txt`, open it, type a sentence, and save it.
2. Hold Shift, right-click `essay.txt`, and choose **Copy as path**.
3. Log in as `stu1` with `Student2026` (after the 15-minute lock from AT-02 is over).
4. Choose `1` (View my assignments), look at the list, then choose `0` (Back).
5. Choose `2` (Submit), then the number of `Essay 1`. Paste the path (right-click in PowerShell pastes) and press Enter. Read the check screen, then answer `y`.
6. Choose `2` again, the number of `Essay 1`, paste the same path, and answer `y`.

Expected: step 4 shows both assignments with their deadlines; step 5 shows "Time left", then a receipt with the file size, version 1, status SUBMITTED and a SHA-256 code; step 6 says "This becomes version 2" and the receipt shows version 2.

Result: ______

## AT-05 Late submission is accepted with a warning (UC16 6a, BR-01)

Stakeholder need: late work is never rejected, and the student knows the penalty before submitting.

Steps:

1. Wait until the `Quick quiz` deadline from AT-03 has passed.
2. Still logged in as `stu1`, choose `2`, the number of `Quick quiz`, paste the path of `essay.txt`, and press Enter.
3. Read the message, then answer `y`.
4. Choose `5` (Log out).

Expected: the message says "This is late by 1 day(s): 10% of the maximum mark will be deducted; zero after 3 days"; the receipt shows status LATE.

Result: ______

## AT-06 Lecturer marks with the penalty shown first (UC11)

Stakeholder need: marks are fair and the late penalty is worked out by the system, not by hand.

Steps:

1. Log in as `lec1` with `Lecturer2026`.
2. Choose `3` (Mark submissions), `1` (CS4433), then the number of `Quick quiz`, then `1` (Thabo Student).
3. Type raw mark `abc`, then `51`, then `40`.
4. For feedback, press Enter with nothing typed, then type `Good answers`.
5. Read the line with the deduction, then answer `y`.
6. The program goes back to the main menu by itself (there is nothing left to mark in `Quick quiz`). Choose `3`, `1`, the number of `Essay 1`, `1`. Raw mark `72`, feedback `Clear and well argued`, answer `y`.
7. Choose `6` (Log out).

Expected: `abc` and `51` are asked again; the empty feedback says "This cannot be empty." and asks again; step 5 shows "Raw 40.0, late deduction 5.0 (1 day(s) late), final 35.0" before you confirm, then "Saved".

Result: ______

## AT-07 HoD moderates and releases; the student sees the result (UC12, UC18)

Stakeholder need: nothing reaches students unchecked, and once released they see mark and feedback.

Steps:

1. Log in as `stu1` with `Student2026`, choose `3` (View released results), then `5` (Log out).
2. Log in as `hod1` with `TempPass1` and change the password to `HeadOfDept2026` (twice).
3. Choose `1` (Moderate marks), the number of `Essay 1`, `1` (Approve all), and answer `y`.
4. Choose `3` (Release marks), the number of `Essay 1`, and answer `n`.
5. Choose `3` again, the number of `Essay 1`, and answer `y`.
6. Choose `4` (View audit log), `1` (All mark changes).
7. Choose `6` (Log out). Log in as `stu1`, choose `3`, then `5`.

Expected: step 1 says no results have been released yet; step 3 shows the mark with its feedback and says 1 mark approved; step 4 changes nothing; step 5 says 1 mark released; step 6 lists the lecturer's marks with who made them; step 7 shows `Essay 1` with mark 72.0 / 100 and the feedback. `Quick quiz` is **not** shown (it was not moderated).

Result: ______

## AT-08 Administrator safety: cancel, backup and restore (UC03, UC08, D14)

Stakeholder need: the administrator can back out of a mistake and recover data.

Steps:

1. Log in as `admin` with `Admin2026!`.
2. Choose `1`, `2`, `1` (Student). At "Username" press Enter without typing anything.
3. Choose `5` (Backup and restore), `1` (Back up now).
4. Choose `1`, `2`, `1`. Username `extra`, full name `Extra Student`, temporary password `TempPass1` twice.
5. Choose `5`, `2` (Restore a backup), `1` (the newest), answer `y`.
6. Choose `7` (Log out), log in as `admin` again, and choose `1`, `1` (List all accounts).

Expected: step 2 goes back to the menu without creating anything; step 3 prints the backup file name; after the restore, step 6 lists the 4 original accounts and **not** `extra`.

Result: ______

---

When all eight say Pass, close the program and delete the test folder by typing this in PowerShell:

```
Remove-Item -Recurse -Force C:\lms-test
```
