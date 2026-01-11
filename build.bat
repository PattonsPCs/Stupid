@echo off
echo Building Random Video...
gcc -o Sutpid.exe main.c gui.c operations.c -luser32 -lgdi32 -lshell32 -mwindows
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Run Sutpid.exe
) else (
    echo Build failed. Make sure you have MinGW or another C compiler installed.
)
pause

