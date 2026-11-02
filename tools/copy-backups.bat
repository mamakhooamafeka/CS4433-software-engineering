@echo off
rem Copies the daily LMS backups to a second place, so one broken disk cannot
rem lose the marks (Phase 1: daily copy to a second location).
rem Windows Task Scheduler runs this every school day (docs\INSTALL.md, step 7).
rem Usage: copy-backups.bat [data folder] [second location]
setlocal
set "DATA=%~1"
if "%DATA%"=="" set "DATA=C:\lms-data"
set "TARGET=%~2"
if "%TARGET%"=="" set "TARGET=E:\lms-backup-copy"

if not exist "%DATA%\backups" (
  echo There is no backups folder in %DATA%. Start the LMS once first.
  exit /b 1
)
robocopy "%DATA%\backups" "%TARGET%" /E /R:1 /W:1 /NP /NJH /LOG+:"%DATA%\backup-copy.log"
rem robocopy exit codes 0 to 7 mean success; 8 or more mean something was not copied.
if %ERRORLEVEL% GEQ 8 (
  echo The copy FAILED. Is the backup drive plugged in? Details: %DATA%\backup-copy.log
  exit /b 1
)
echo Backups copied to %TARGET%.
exit /b 0
