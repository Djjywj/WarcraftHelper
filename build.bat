@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ================================================
echo   WarcraftHelper build (with cursor lock)
echo ================================================
echo.

set "CMAKE_EXE="
where cmake >nul 2>nul
if not errorlevel 1 set "CMAKE_EXE=cmake"

if not defined CMAKE_EXE (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -property installationPath`) do set "VSPATH=%%i"
    )
    if defined VSPATH (
        set "CMAKE_EXE=!VSPATH!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    )
)

if not defined CMAKE_EXE (
    echo [ERROR] cmake not found.
    echo.
    echo Option 1: In Visual Studio Installer, click Modify, enable
    echo           "Desktop development with C++", then rerun this script.
    echo Option 2: Install CMake from https://cmake.org/download/
    echo           (check "Add CMake to the system PATH")
    echo.
    pause
    exit /b 1
)
echo Using cmake: !CMAKE_EXE!
echo.

echo [1/3] Generating project files...
"!CMAKE_EXE!" . -A win32 -B build
if errorlevel 1 (
    echo.
    echo [ERROR] Configure failed. Install "Desktop development with C++" in Visual Studio Installer.
    echo.
    pause
    exit /b 1
)

echo.
echo [2/3] Building... this may take a few minutes, do not close this window.
"!CMAKE_EXE!" --build build --config MinSizeRel
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. Send me a screenshot of the red text above.
    echo.
    pause
    exit /b 1
)

echo.
echo.
echo.
echo [3/3] Collecting the files you need...
if exist "ready" rd /s /q "ready"
mkdir "ready"

for /r "build" %%f in (WarcraftHelper.dll) do copy /Y "%%f" "ready\" >nul
for /r "build" %%f in (WarcraftHelperLoader.mix) do copy /Y "%%f" "ready\" >nul
for /r "build" %%f in (d3d9.dll) do copy /Y "%%f" "ready\" >nul
copy /Y "WarcraftHelper.ini" "ready\" >nul

echo.
echo Build finished.
echo.
echo [Warnings]
set "BAD=0"
if not exist "ready\WarcraftHelper.dll" (echo [WARN] WarcraftHelper.dll not found & set "BAD=1")
if not exist "ready\WarcraftHelperLoader.mix" (echo [WARN] WarcraftHelperLoader.mix not found & set "BAD=1")
if not exist "ready\d3d9.dll" (echo [WARN] d3d9.dll not found & set "BAD=1")
if not exist "ready\WarcraftHelper.ini" (echo [WARN] WarcraftHelper.ini not found & set "BAD=1")
if "!BAD!"=="1" echo [NOTE] Some files are missing, the build probably failed. Send me the red text above.
echo.
echo Copy the 4 files in the "ready" folder:
dir /b "ready"
echo.
echo Put them into your Warcraft III folder, overwrite when asked.
echo.
echo (.exp .lib .pdb in the build folder are leftovers, safe to ignore)
echo.
pause
