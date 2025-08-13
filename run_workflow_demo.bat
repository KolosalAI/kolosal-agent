@echo off
echo === Kolosal Agent Workflow Engine Demo ===
echo.

REM Check if the executable exists
if not exist "build\Debug\workflow-example.exe" (
    echo Error: workflow-example.exe not found in build\Debug\
    echo Please build the project first using CMake.
    echo.
    echo To build:
    echo   mkdir build
    echo   cd build
    echo   cmake ..
    echo   cmake --build . --config Debug
    echo.
    pause
    exit /b 1
)

echo Starting workflow engine demo...
echo.

REM Run the workflow example
"build\Debug\workflow-example.exe"

echo.
echo Demo completed.
pause
