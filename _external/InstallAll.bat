@echo off
REM Copy all external libraries.

xcopy "bins\Win32" "..\Debug\" /E /C
xcopy "bins\Win32" "..\Release\" /E /C
