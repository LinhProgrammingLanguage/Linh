@echo off
setlocal enabledelayedexpansion

REM Detect number of CPU cores for parallel build
for /f "skip=2 tokens=2 delims== " %%A in ('wmic cpu get NumberOfLogicalProcessors /value') do set NUM_CORES=%%A
if not defined NUM_CORES set NUM_CORES=4

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

REM Configure CMake for Release
cmake -DCMAKE_BUILD_TYPE=Release ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    cd "%ROOT_DIR%"
    exit /b %errorlevel%
)

REM Build project in Release mode, parallel
cmake --build . --config Release -- /m:%NUM_CORES%
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

REM Automatically run Release executable if build succeeded
set EXE_PATH=%BUILD_DIR%\Release\LinhApp.exe
if exist "%EXE_PATH%" (
    echo Running Release build: %EXE_PATH%
    "%EXE_PATH%"
) else (
    echo [WARNING] Release executable not found: %EXE_PATH%
)

endlocal