# Lecturer manual

Moderated LMS v1.1.0. This manual is for **lecturers**. It shows how to set assignments, give extensions, mark work and correct a mark.

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
Main menu: Mr Lecturer (LECTURER)
---------------------------------
  You have 1 submission(s) to mark.
  1. Create an assignment
  2. Grant an extension
  3. Mark submissions
  4. Correct a mark
  5. Change my password
  6. Log out
Choose:
```

The line above the options shows how many submissions in your courses are waiting for a mark. You only see the courses the administrator has assigned to you. If a course is missing, ask the administrator.

## 3. Create an assignment

1. Choose `1. Create an assignment`, then the number of the course.
2. Type the title, the instructions, the deadline and the maximum mark (from 1 to 1000). Remember that the deadline is in UTC: subtract 2 hours from Lesotho time.
   ```
   New assignment for CS4433
   -------------------------
   Title: Essay 1
   Instructions: Write 500 words on software testing.
   Deadline (YYYY-MM-DD HH:MM, UTC): YYYY-MM-DD 12:00
   Maximum mark (1-1000): 100
   Create "Essay 1" due YYYY-MM-DD 12:00? (y/n): y
     The assignment has been created. Enrolled students can see it now.
   ```
3. Check the summary line and type `y`. Every student enrolled in the course can see the assignment straight away.

## 4. Give an extension

An extension gives more time. You can give it to **one student** (for example after an illness) or to **the whole class** (for example when the school was closed). An extension can only make a deadline later, never earlier, and it always needs a reason.

### 4.1 One student

Use this when a student has a good reason, such as illness. The extension is only for that student; the deadline stays the same for everyone else.

1. Choose `2. Grant an extension`.
2. Choose the course and the assignment.
3. At `Who gets the extension?` choose `1. One student`, then choose the student.
4. Type the new deadline (UTC) and the reason. The new deadline must be later than the deadline the student has now.
   ```
   New deadline (YYYY-MM-DD HH:MM, UTC): YYYY-MM-DD 12:00
   Reason: Was ill, doctor's note seen
   Extend "Essay 1" for Thabo Student to YYYY-MM-DD 12:00? (y/n): y
     The extension has been granted.
   ```
5. The student now sees the new deadline marked `(ext. you)`. The reason is **not** shown to the student, because it may be private. If the student already handed in late, the program works out again whether the work is late using the new deadline. Work that is already marked cannot be given an extension.

### 4.2 The whole class

Use this when the deadline has to move for everyone, for example when the school was closed, or when you typed the wrong deadline when you created the assignment.

1. Choose `2. Grant an extension`.
2. Choose the course and the assignment.
3. At `Who gets the extension?` choose `2. The whole class`. The program shows the deadline the class has now.
4. Type the new deadline (UTC) and the reason. **Every student in the class sees this reason**, so write it for them.
   ```
   Who gets the extension?
   -----------------------
     1. One student
     2. The whole class
     0. Back
   Choose: 2
     The deadline for the whole class is now YYYY-MM-DD 12:00 UTC.
   New deadline (YYYY-MM-DD HH:MM, UTC): YYYY-MM-DD 12:00
   Reason (the students will see it): School closed for the inter-schools sports day
   Extend "Essay 2" for the whole class of CS4433 to YYYY-MM-DD 12:00? (y/n): y
     The extension has been granted to all 1 student(s) of CS4433.
   ```
5. Every student of the course, including students enrolled later, now sees the new deadline marked `(ext. class)`, with your reason.

What else happens:

- Work that was handed in late, but before the new deadline, is now on time. The program says how many: `2 late submission(s) are now on time.`
- A student who already has a **longer** extension of their own keeps it.
- Work that you have **already marked** keeps its mark. If such a mark has a late deduction that is no longer fair, the program tells you how many, for example `1 marked submission(s) still have a late deduction although they are on time now.` Correct those marks with `4. Correct a mark` (section 6): the program works the deduction out again with the new deadline. If the head of department has already moderated the mark, ask them to amend it.
- After the head of department has released the marks of an assignment, it cannot be extended any more.

## 5. Mark submissions

1. Choose `3. Mark submissions`, then the course and the assignment.
2. You see the submissions that are not marked yet, with the student's name, the status, the version number and when it was handed in. Late work also shows how many days late it is:
   ```
   Unmarked submissions for Essay 1
   --------------------------------
     1. Thabo Student | SUBMITTED | v1 YYYY-MM-DD 17:16
     0. Back
   ```
3. Type the number of a submission. The program shows where the file is stored, for example:
   ```
     File: C:\lms-data\submissions\1\4\v1_Essay1.docx
   ```
   Open that file in File Explorer (it opens in Word or the right program) and read the work.
4. Type the **raw mark**: the mark for the quality of the work, before any late penalty. Then type your feedback. Feedback cannot be empty.
5. The program works out the late deduction for you and shows the final mark **before** saving:
   ```
   Raw mark (0-100): 72
   Feedback: Good structure; add more examples of unit tests.
     Raw 72.0, late deduction 0.0 (0 day(s) late), final 72.0.
   Save this mark? (y/n): y
     Saved. Students see it only after the HoD releases it.
   ```
   Type `y` to save it, or `n` to type the mark again.
6. The list comes back with the next unmarked submission. Type `0` when you are finished.

You do not need to work out late penalties yourself. The rule is the same for every student: 10% of the maximum mark is taken off for each day late (or part of a day), and work more than 3 days late gets 0.

Students cannot see your marks yet. The head of department checks (moderates) them and then releases them.

## 6. Correct a mark

You can correct a mark you gave **until the head of department has moderated it**.

1. Choose `4. Correct a mark`, then the course and the assignment.
2. Choose the submission. The program shows the current mark and feedback.
3. Type the corrected raw mark, new feedback (or just press Enter to keep the old feedback), and the reason for the correction.
4. Type `y` to save. The program shows the new final mark.

Every correction is written to the audit log with your name, the old and new mark, and your reason. The head of department can see it.

After moderation, only the head of department can change a mark. Ask them if a moderated mark is wrong.

## 7. Problems

| Problem | What to do |
|---|---|
| My course is not in the list | The administrator has not assigned you to it yet |
| A student is not in the list for an extension | The student is not enrolled in the course. Ask the administrator |
| The deadline is too early (for example 2 hours wrong because you typed Lesotho time instead of UTC) | Give the whole class an extension to the right time (section 4.2) |
| `An extension can only move the deadline later (BR-02).` | The new deadline must be later than the deadline in force now. A deadline cannot be made earlier |
| `Marks for this assignment are released, so it cannot be extended (BR-06).` | The assignment is closed. Nothing can be handed in any more |
| The submission I want to correct is not in the list | It is already moderated, or not marked yet. Ask the head of department |
| I forgot my password, or my account is locked | Wait 15 minutes if it is locked, or ask the administrator to reset it |
