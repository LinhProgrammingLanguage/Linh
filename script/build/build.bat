@echo off
setlocal enabledelayedexpansion

echo +-------------------------------+
echo   Build project in Release mode
echo +-------------------------------+
echo.

REM Set build directory
set BUILD_DIR=build

REM Save current directory
set ROOT_DIR=%CD%

REM Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

REM Change to build directory
cd "%BUILD_DIR%"

REM Configure CMake (not using vcpkg toolchain)
cmake ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    cd "%ROOT_DIR%"
    exit /b %errorlevel%
)

REM Build project in Release mode
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    cd "%ROOT_DIR%"
    exit /b %errorlevel%
)

REM Return to root directory
cd "%ROOT_DIR%"

echo +-------------------------------+
echo   Build completed successfully!
echo +-------------------------------+
echo.

REM Automatically run Debug executable if build succeeded
set EXE_PATH=%BUILD_DIR%\Release\LinhApp.exe
if exist "%EXE_PATH%" (
    echo Running Release build: %EXE_PATH%
    "%EXE_PATH%"
) else (
    echo [WARNING] Release executable not found: %EXE_PATH%
)

endlocal