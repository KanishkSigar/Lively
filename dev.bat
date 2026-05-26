@echo off
REM Loads CMake, Ninja, and modern MinGW g++ (WinLibs UCRT) into the current
REM cmd session's PATH. Run this once per cmd window before building.
REM
REM Usage:
REM   dev
REM   cmake --workflow --preset ci

set "PATH=%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin;C:\Program Files\CMake\bin;%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;%PATH%"

echo LiveLy dev environment loaded.
where cmake >nul 2>nul || echo   [missing] cmake  -- install with: winget install Kitware.CMake
where ninja >nul 2>nul || echo   [missing] ninja  -- install with: winget install Ninja-build.Ninja
where g++   >nul 2>nul || echo   [missing] g++    -- install with: winget install BrechtSanders.WinLibs.POSIX.UCRT
echo.
echo Next:  cmake --workflow --preset ci
