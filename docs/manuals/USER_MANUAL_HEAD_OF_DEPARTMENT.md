# Head of department manual

Moderated LMS v1.1.0. This manual is for the **head of department** (HoD). It shows how to check (moderate) the marks in your department, change a mark when needed, release marks to students, and read the audit log.

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

## 2. Your main menu

```
Main menu: Dr Head (HOD)
------------------------
  You have 1 submission(s) to moderate.
  1. Moderate marked submissions
  2. Amend a mark
  3. Release marks for an assignment
  4. View the audit log
  5. Change my password
  6. Log out
Choose:
```

The line above the options shows how many marked submissions in your department are waiting for you. You see only the courses of your own department.

How a mark moves through the system:

| Step | Who | Status afterwards | Can students see the mark? |
|---|---|---|---|
| Student hands in | Student | SUBMITTED or LATE | No |
| Lecturer marks | Lecturer | MARKED | No |
| HoD approves (section 3) | You | MODERATED | No |
| HoD releases (section 5) | You | RELEASED | **Yes** |

## 3. Moderate (approve) marks

1. Choose `1. Moderate marked submissions`.
2. Choose the assignment. Each line shows how many submissions are marked, moderated, released and not yet marked:
   ```
   Assignments in my department
   ----------------------------
     1. CS4433 Essay 1 | marked 1, moderated 0, released 0, unmarked 0
     0. Back
   ```
3. You see every marked submission with the raw mark, the late deduction, the final mark, the days late and the lecturer's feedback:
   ```
   Marked submissions: Essay 1
   ---------------------------
     1. Thabo Student | MARKED | raw 72.0 - 0.0 = 72.0 | Good structure; add mor...
   ```
4. Check that the marks are fair and consistent. If one is wrong, do not approve it yet: amend it first (section 4), or ask the lecturer to correct it.
5. Choose `1. Approve all`, or `2. Approve some` and type the numbers of the ones to approve, separated by spaces (for example `1 3 4`).
6. Type `y` to confirm. The approved marks become MODERATED. The lecturer can no longer change them.

## 4. Amend a mark

Use this when a mark is wrong after moderation, or when you disagree with the lecturer's mark.

1. Choose `2. Amend a mark`, then the assignment and the submission.
2. Type the new **raw** mark. The program takes off the same late deduction again, so you never have to work it out.
3. Type the reason. A reason is required.
4. Type `y` to confirm. The program shows the new final mark.

If the mark was already released, the program warns you first, and the student sees the new mark straight away. Every amendment is written to the audit log with your name, the old and new mark, and the reason.

## 5. Release marks

1. Choose `3. Release marks for an assignment`, then the assignment.
2. The program says how many moderated marks will be released, and how many submissions are not moderated yet:
   ```
   Release marks: Essay 1
   ----------------------
     1 moderated mark(s) will be released.
     Students see released marks at once, and the assignment closes (BR-06).
   Release now? (y/n):
   ```
3. Type `y`. All moderated marks for that assignment are released together: either all of them, or none if something goes wrong. Students can see them at once.

After release, the assignment is **closed**: students can no longer hand in to it. Submissions that were not moderated stay hidden from students. Moderate them and release again later.

## 6. View the audit log

The audit log is a record of every mark that was given or changed in your department. Nobody can change or delete it, not even the administrator.

1. Choose `4. View the audit log`.
2. Choose what to see:
   ```
     1. All mark changes in my department
     2. Only one course
     3. Only one assignment
     4. Only one student (by number)
   ```
   For a course or an assignment, choose the assignment from the list. For one student, type their user number (the administrator can see it in the list of accounts).
3. Each entry shows when the change happened, what kind of change it was, the old and new raw/final mark, who made it, and the reason:
   ```
   Audit log (1 entries)
   ---------------------
     YYYY-MM-DD 17:16 INITIAL submission 1: - -> 72.0/72.0
         by Mr Lecturer, reason: Initial mark
   ```

| Kind | Meaning |
|---|---|
| INITIAL | The lecturer's first mark |
| CORRECTION | The lecturer corrected the mark before moderation |
| AMENDMENT | The HoD changed the mark |

Use the audit log to answer a student's or parent's question about a mark.

## 7. Problems

| Problem | What to do |
|---|---|
| An assignment is not in my list | It belongs to a course outside your department, or no course in your department has assignments yet |
| `Release now?` does not appear | No marks are moderated yet. Moderate first (section 3) |
| A student says their mark is missing | Their submission may not be moderated yet, or not marked. Check the counts in the assignment list |
| I forgot my password, or my account is locked | Wait 15 minutes if it is locked, or ask the administrator to reset it |
