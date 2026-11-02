@echo off
rem Starts the Moderated LMS with the school's data folder.
rem The installation guide (docs\INSTALL.md) puts a desktop shortcut to this file.
rem To keep the data somewhere else, change C:\lms-data below.
title Moderated LMS
cd /d "%~dp0"
lms.exe "C:\lms-data"
echo.
pause
