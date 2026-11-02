# Student manual

Moderated LMS v1.1.0. This manual is for **students**. It shows how to see your assignments, hand in your work, and read your marks.

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

After you log in you see your main menu. If you have assignments that you have not handed in yet, the line above the options tells you how many:

```
Main menu: Thabo Student (STUDENT)
----------------------------------
  You have 1 assignment(s) to submit.
  1. View my courses and assignments
  2. Submit an assignment
  3. View my released results
  4. Change my password
  5. Log out
Choose:
```

## 3. See your assignments and deadlines

1. Choose `1. View my courses and assignments`.
2. You see every assignment in your courses, with its deadline and what has happened to your work:
   ```
     ext. class = deadline extended for the whole class
     ext. you   = deadline extended for you only

   My assignments (choose one to see the instructions)
   ---------------------------------------------------
     1. CS4433 Essay 1 | due YYYY-MM-DD 12:00 (ext. you) | SUBMITTED
     2. CS4433 Essay 2 | due YYYY-MM-DD 12:00 (ext. class) | not submitted
     0. Back
   ```
   The deadline shown is **your own** deadline, with any extension already included:
   - `(ext. class)` means your lecturer gave the whole class more time.
   - `(ext. you)` means your lecturer gave only you more time.
   - No mark means the deadline is the one the assignment was set with.
3. Type the number of an assignment to read its instructions and maximum mark. If the deadline was extended, you also see the original deadline, and for a class extension the lecturer's reason:
   ```
   Essay 2
   -------
     Course: CS4433 Software Engineering
     Deadline: YYYY-MM-DD 12:00 UTC
     Extended for the whole class from YYYY-MM-DD 12:00 UTC.
     Reason: School closed for the inter-schools sports day
     Maximum mark: 100
     Instructions: Compare black-box and white-box testing in 800 words.
   ```
   The reason for an extension given only to you is not shown on the screen, because other people may be looking at the same computer.

What the status at the end of each line means:

| Status | Meaning |
|---|---|
| not submitted | You have not handed in anything yet |
| SUBMITTED | Handed in on time |
| LATE | Handed in after your deadline; a late penalty applies (section 5) |
| MARKED | Your lecturer has marked it. You cannot see the mark yet |
| MODERATED | The head of department has checked the mark. You cannot see it yet |
| RELEASED | Your mark is ready: see section 6 |
| closed | The marks for this assignment have been released and it no longer takes submissions |

## 4. Hand in an assignment

Have your file ready on the computer or on your USB flash drive. It can be any kind of file (for example Word or PDF), up to 10 MB.

1. Choose `2. Submit an assignment`, then type the number of the assignment.
2. The program asks for the full path of your file:
   ```
   Full path of your file:
   ```
   The easiest way to get the path: open File Explorer, find your file, hold **Shift**, right-click the file, and choose **Copy as path**. Then click in the black window, right-click once to paste, and press Enter. For example, a file on a USB flash drive might be `E:\Essay1.docx`.
3. The program checks the file. If it cannot find the file, or the file is empty or larger than 10 MB, it says so and lets you try again (3 tries).
4. Before anything is saved, the program shows a check screen:
   ```
   Check before you submit
   -----------------------
     Assignment: Essay 1
     Your deadline: YYYY-MM-DD 12:00 UTC
     Time now:      YYYY-MM-DD 17:16 UTC
     Time left:     23 day(s) 18 h 43 min
   Submit this file now? (y/n):
   ```
   Read it carefully. Type `y` to hand in the file, or `n` to stop without handing anything in.
5. You get a receipt:
   ```
   Submission receipt
   ------------------
     Assignment: Essay 1
     File: Essay1.docx (24 bytes)
     Version: 1, status SUBMITTED
     Received: YYYY-MM-DD 17:16 UTC
     SHA-256: 180ee8fe4b99d15d3543a7f9574c691ba9ba24a829999b5b626169e1015439d4
   ```
   **Write down or photograph the receipt.** It proves when you handed in and which file you handed in: the long SHA-256 code changes if even one letter of the file is different.

### Hand in a new version

You can hand in again, as many times as you like, **until your deadline** and as long as the work has not been marked. Do the same steps. The check screen says `This becomes version 2.` Your earlier versions are kept, but only the newest one is marked.

After the deadline, or after marking, the program does not allow a new version. It tells you why and shows the version it kept.

## 5. Handing in late

If your deadline has passed, the check screen warns you before you hand in:

```
  This is late by 1 day(s): 10% of the maximum mark will be deducted; zero after 3 days.
```

The late penalty is the same for everyone and is worked out by the program, not by the lecturer:

| How late | Deduction from your mark |
|---|---|
| Up to 1 day (even 1 minute) | 10% of the maximum mark |
| More than 1 day, up to 2 days | 20% of the maximum mark |
| More than 2 days, up to 3 days | 30% of the maximum mark |
| More than 3 days | The mark becomes 0 |

If you have a good reason for being late (for example illness), talk to your lecturer **before** the deadline. Only the lecturer can give you an extension, and it then shows as `(ext. you)` in your list. If the whole class gets more time, it shows as `(ext. class)`. If you handed in late but before the new deadline, your work counts as on time.

## 6. See your marks

1. Choose `3. View my released results`.
2. You see each released mark, how it was worked out, and your lecturer's feedback:
   ```
   My released results
   -------------------

     CS4433 Essay 1
     Mark: 72.0 / 100  (raw 72.0, late deduction 0.0, days late 0)
     Feedback: Good structure; add more examples of unit tests.
   ```
   `raw` is the mark your lecturer gave; the late deduction is taken off it to give your final mark.

You only see a mark after the head of department has checked and released it. Until then the screen says `No results have been released yet.` This is the school's rule, so that every class gets checked, fair marks at the same time.

## 7. Change your password

Choose `4. Change my password`. Type your current password, then your new one twice. Change it straight away if you think someone else knows it.

## 8. Problems

| Problem | What to do |
|---|---|
| I forgot my password, or my account is locked | Wait 15 minutes if it is locked, or ask the administrator to give you a new temporary password |
| My assignment is not in the list | You may not be enrolled in that course. Ask your lecturer or the administrator |
| `That file does not exist. Check the path and try again.` | Check the path. Use **Copy as path** (section 4, step 2) so it is exactly right. If your USB flash drive was unplugged, plug it in again |
| `That file is larger than 10 MB.` | Make it smaller than 10 MB (for example, save pictures in a smaller size) and try again |
| `That file cannot be read. Is it open in another program?` | Close the file in Word (or the other program) and try again |
| I cannot hand in a new version | Your deadline has passed or your work is already marked. Talk to your lecturer |
| The time looks wrong | The program uses UTC, 2 hours behind Lesotho time. If it is still wrong, tell the administrator: the computer clock may be wrong |
