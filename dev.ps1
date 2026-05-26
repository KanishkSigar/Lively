# Loads CMake, Ninja, and modern MinGW g++ (WinLibs UCRT) into the current
# PowerShell session's PATH. Must be DOT-SOURCED so PATH changes persist:
#
#   . .\dev.ps1
#   cmake --workflow --preset ci
#
# Without the leading dot, the script runs in a child scope and PATH reverts.

$winlibs = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
$cmakeBin = 'C:\Program Files\CMake\bin'
$ninjaBin = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"

$env:Path = "$winlibs;$cmakeBin;$ninjaBin;$env:Path"

Write-Host 'LiveLy dev environment loaded.'
foreach ($pair in @(
        @{Name='cmake'; Install='winget install Kitware.CMake'},
        @{Name='ninja'; Install='winget install Ninja-build.Ninja'},
        @{Name='g++';   Install='winget install BrechtSanders.WinLibs.POSIX.UCRT'})) {
    if (-not (Get-Command $pair.Name -ErrorAction SilentlyContinue)) {
        Write-Host "  [missing] $($pair.Name)  -- install with: $($pair.Install)"
    }
}
Write-Host ''
Write-Host 'Next:  cmake --workflow --preset ci'
