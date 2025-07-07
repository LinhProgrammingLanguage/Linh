@echo off
setlocal
set EXE_PATH=build\Debug\LinhApp.exe
if exist "%EXE_PATH%" (
    echo Running: %EXE_PATH%
    "%EXE_PATH%"
) else (
    set EXE_PATH=build\Release\LinhApp.exe
    if exist "%EXE_PATH%" (
        echo Running: %EXE_PATH%
        "%EXE_PATH%"
    ) else (
        echo [ERROR] LinhApp.exe not found in Debug or Release folder!
    )
)
endlocal
