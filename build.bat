@echo off
echo Building Random Video for 64-bit Windows...
echo.
gcc -m64 -o Sutpid.exe main.c gui.c operations.c dependencies.c -luser32 -lgdi32 -lshell32 -mwindows -static-libgcc
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! Run Sutpid.exe
    echo This executable is built for 64-bit Windows and should work on modern PCs.
) else (
    echo.
    echo Build failed. Make sure you have MinGW 64-bit compiler installed.
    echo If you only have a 32-bit compiler, you need to install a 64-bit version.
)
pause

