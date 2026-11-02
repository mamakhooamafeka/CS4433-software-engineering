# Installation guide for the school technician

Moderated LMS v1.1.0 for Windows. This guide shows how to install the program on the school office computer, how to set up the daily backup copy, and how to check that everything works. You do not need to know programming. Every step says exactly what to click or type.

Where things go after installation:

| What | Where |
|---|---|
| The program | `C:\ModeratedLMS` |
| All school data (database, submitted files, daily backups, error log) | `C:\lms-data` |
| The second copy of the backups | `E:\lms-backup-copy` on an external USB hard disk |

The deployment diagram in `docs/DEPLOYMENT.md` shows the same set-up as a picture.

---

## 1. Before you start

Check that you have these things:

| Item | Requirement |
|---|---|
| School office computer | Windows 10 or Windows 11, 64-bit. At least 4 GB of memory and 2 GB of free disk space. No internet connection is needed when the program runs |
| A Windows account with administrator rights | Needed only for installing and for setting up the scheduled backup copy |
| A USB flash drive | To carry the program from a computer that has internet |
| An external USB hard disk | For the second copy of the backups. Recommended: without it, a broken computer disk loses all marks |
| The school administrator | The person who will manage accounts. They choose the first administrator password in step 6 |

To check the Windows version: press the Windows key, type `about`, open **About your PC**, and look at **System type**. It must say **64-bit operating system**.

---

## 2. Download the program

Do this on any computer that has internet.

1. Open https://github.com/mamakhooamafeka/CS4433-software-engineering/releases in a web browser.
2. Under **Moderated LMS v1.1.0**, open **Assets** and click `ModeratedLMS-1.1.0-windows-x64.zip`. The file is saved to the Downloads folder.
3. Copy the zip file to the USB flash drive and take it to the school office computer.

---

## 3. Install the program

On the school office computer:

1. Plug in the USB flash drive and copy `ModeratedLMS-1.1.0-windows-x64.zip` to the Desktop.
2. Right-click the zip file and choose **Extract All...**.
3. In the box **Files will be extracted to this folder**, type `C:\` and click **Extract**. This makes the folder `C:\ModeratedLMS-1.1.0-windows-x64`.
4. Rename that folder to `ModeratedLMS`: open **This PC > Local Disk (C:)**, right-click the folder, choose **Rename**, type `ModeratedLMS` and press Enter.
5. Open `C:\ModeratedLMS`. It must contain:

| File or folder | What it is |
|---|---|
| `lms.exe` | The program |
| `sqlite3.dll`, `libsodium.dll` | Libraries for the database and for password protection |
| `msvcp140.dll`, `vcruntime140.dll` and other `*140*.dll` files | The Microsoft Visual C++ runtime the program needs |
| `start-lms.bat` | Starts the program with the school data folder |
| `copy-backups.bat` | Copies the backups to the external disk |
| `docs\` | This guide, the four user manuals and the release notes |
| `licences\` | Licence texts of the libraries |

---

## 4. Make a desktop shortcut

1. In `C:\ModeratedLMS`, right-click `start-lms.bat`. On Windows 11 choose **Show more options** first.
2. Choose **Send to > Desktop (create shortcut)**.
3. On the Desktop, right-click the new shortcut, choose **Rename**, type `Moderated LMS` and press Enter.

The shortcut always uses the data folder `C:\lms-data`, wherever it is started from. Do not start `lms.exe` directly by double-clicking it, because then the program keeps its data in a new folder next to wherever it was started, and the school data seems to disappear (defect D15).

---

## 5. Set the Windows clock and time zone

The program decides whether a submission is late from the computer's clock, so the clock must be right.

1. Press the Windows key, type `date and time`, and open **Date & time settings**.
2. Set **Time zone** to **(UTC+02:00) Harare, Pretoria** (the same time as Lesotho).
3. Check the date and the time. If they are wrong, turn off **Set time automatically**, click **Change**, and set them.
4. Check the clock again once a week. The computer is offline, so Windows cannot correct it by itself.

The program shows all times in UTC, which is 2 hours behind Lesotho time. The user manuals explain this to the users.

---

## 6. First start and the administrator account

Do this together with the school administrator, because they choose the administrator password.

1. Double-click **Moderated LMS** on the Desktop. A black window opens with:
   ```
   Moderated LMS v1.1. All times are UTC.
     Today's backup has been made.

   First start: create the administrator account
   ---------------------------------------------
   Username:
   ```
2. Let the school administrator type a username (for example `admin`), their full name, and a password of at least 8 characters, twice. The password is hidden while it is typed.
3. The program says `Administrator created. Please log in.` Let the administrator log in once to check it works. Then choose `7. Log out`.
4. At the `Username:` prompt, press Enter without typing anything. The program says `Goodbye.` Press any key to close the window.
5. Open `C:\lms-data`. It must now contain `lms.db`, `submissions` and `backups`.

The administrator now follows the **Administrator manual** (`docs\USER_MANUAL_ADMINISTRATOR.md`) to create the other accounts.

### Larger text (for users who need it)

1. Start the program with the shortcut.
2. Right-click the title bar of the black window and choose **Properties**.
3. On the **Font** tab, choose size **20** or larger and click **OK**. Windows remembers this for the shortcut.

---

## 7. Daily second copy of the backups

The program makes one backup inside `C:\lms-data\backups` the first time it starts each day. If the computer's disk breaks, those backups are lost with it, so a second copy goes to the external USB hard disk every school day.

### 7.1 Test the copy once by hand

1. Plug the external USB hard disk into the computer and leave it plugged in.
2. Open **This PC** and note the drive letter of the disk, for example `E:`. The commands below use `E:`. If your disk has another letter, use that letter instead.
3. Press the Windows key, type `cmd`, and open **Command Prompt**.
4. Type this line and press Enter:
   ```
   C:\ModeratedLMS\copy-backups.bat C:\lms-data E:\lms-backup-copy
   ```
5. It must end with `Backups copied to E:\lms-backup-copy.` Open `E:\lms-backup-copy` and check that it has a folder for today's backup.

### 7.2 Make Windows do it every school day

1. Press the Windows key, type `cmd`, right-click **Command Prompt** and choose **Run as administrator**.
2. Type this line and press Enter. It is one long line:
   ```
   schtasks /Create /TN "Moderated LMS backup copy" /TR "C:\ModeratedLMS\copy-backups.bat C:\lms-data E:\lms-backup-copy" /SC WEEKLY /D MON,TUE,WED,THU,FRI /ST 16:30 /F
   ```
   It must say `SUCCESS: The scheduled task "Moderated LMS backup copy" has successfully been created.`
3. Test the task now. Type this line and press Enter:
   ```
   schtasks /Run /TN "Moderated LMS backup copy"
   ```
4. Open `C:\lms-data\backup-copy.log`. The newest lines show what was copied.

The task runs at 16:30 when someone is logged in to Windows. If the computer is switched off at 16:30, the copy happens the next school day, and nothing is lost because every daily backup is copied.

---

## 8. Check the installation

Tick each check. All must pass before the system is used for real work.

| # | Check | Expected result |
|---|---|---|
| 1 | Start the program from the Desktop shortcut | The log-in screen appears; no message about a missing DLL |
| 2 | Log in as the administrator | The administrator main menu with 7 options |
| 3 | Choose `5. Back up or restore`, then `1. Back up now` | `Backup saved as ...` |
| 4 | Log in with a wrong password 5 times on a test account | The account is locked for 15 minutes |
| 5 | Open `C:\lms-data` | `lms.db`, `submissions`, `backups` are there |
| 6 | Open `E:\lms-backup-copy` after step 7 | The same backup folders are there |
| 7 | Close the window with the X, start again | The data is still there; the program says the database is fine (no "damaged" message) |

---

## 9. Restore after a broken computer or disk

1. Install the program again on the repaired or new computer (steps 2 to 5).
2. Create the folder `C:\lms-data\backups`.
3. From `E:\lms-backup-copy`, copy the newest backup folder (the folder names start with the date, so the newest is last in the list) into `C:\lms-data\backups`.
4. Start the program. Because the database is new and empty, it asks for a first administrator. Create a temporary one, for example username `restore`.
5. Log in as `restore`, choose `5. Back up or restore`, then `2. Restore a backup`. Choose the backup you copied, and type `y`. Be careful to pick the folder you copied in step 3: the program also made a backup of the new, empty database when it started, and that one is no use.
6. Log out. All accounts and data from the backup are back, and the temporary `restore` account is gone, because the restored database replaces the new one. Log in with the school administrator's normal account.

---

## 10. Install a new version

1. Read the release notes of the new version first (`docs\RELEASE_NOTES.md` in its zip).
2. Ask everyone to log out, then let the administrator make a backup (`5. Back up or restore`, `1. Back up now`).
3. Close the program.
4. Rename `C:\ModeratedLMS` to `C:\ModeratedLMS-old`.
5. Install the new version into `C:\ModeratedLMS` (steps 2 and 3). Do not touch `C:\lms-data`.
6. Start the program and log in. If it works, delete `C:\ModeratedLMS-old`. If not, rename the folders back and contact support.

The first time a new version starts, it upgrades the database in `C:\lms-data` by itself if the new version needs new tables (v1.1.0 adds the table for class extensions). The upgrade keeps all data and happens in one step: if the power fails during it, the database stays as it was and the upgrade runs again at the next start. An older version should not be used again on an upgraded database; to go back, restore the backup made in step 2. A backup from an older version can be restored in the new version: it is upgraded in the same way.

---

## 11. Remove the program

1. Delete the Desktop shortcut and the folder `C:\ModeratedLMS`.
2. Delete the scheduled task: in a Command Prompt run as administrator, type `schtasks /Delete /TN "Moderated LMS backup copy" /F`.
3. Do not delete `C:\lms-data` or the external disk until the school has decided how long marks must be kept. They hold learners' personal data, so destroy them safely when they are no longer needed.

---

## 12. Problems and fixes

| What you see | What to do |
|---|---|
| `The code execution cannot proceed because VCRUNTIME140.dll was not found` (or `MSVCP140.dll`) | On a computer with internet, download https://aka.ms/vs/17/release/vc_redist.x64.exe, copy it to the school computer, double-click it and click **Install**. Then start the program again |
| `sqlite3.dll` or `libsodium.dll` not found | The program was started from the wrong folder, or a file was not extracted. Extract the zip again (step 3) and use the shortcut |
| The program asks for a new first administrator and all data seems gone | The program was started without the data folder. Close it, and start it only with the Desktop shortcut. The data is still in `C:\lms-data` |
| `The database file is damaged. Please restore a backup.` | Log in as administrator and restore the newest backup (`5. Back up or restore`, `2. Restore a backup`). If you cannot log in, follow section 9 |
| `copy-backups.bat` says `The copy FAILED` | The external disk is not plugged in, or it has another drive letter. Plug it in, check the letter, and if needed create the task again with the right letter (section 7.2) |
| Submissions are marked late when they were on time | The Windows clock or time zone is wrong (section 5) |
| A user forgot their password or is locked out | The administrator resets it (Administrator manual). A lock also ends by itself after 15 minutes |
| Something else goes wrong | The screen shows a short message. The details are in `C:\lms-data\lms-error.log`. Send that file to support |
