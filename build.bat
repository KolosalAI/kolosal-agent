@echo off
setlocal enabledelayedexpansion

REM Kolosal Agent Build Script for Windows
REM This script builds the Kolosal Agent system with the integrated server

REM Script directory
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%"

REM Default values
set "BUILD_TYPE=Release"
set "BUILD_DIR=build"
set "CLEAN_BUILD=false"
set "VERBOSE=false"
set "JOBS=%NUMBER_OF_PROCESSORS%"
set "USE_CUDA=OFF"
set "USE_VULKAN=OFF"
set "USE_METAL=OFF"
set "ENABLE_NATIVE_OPT=OFF"

REM Parse command line arguments
:parse_args
if "%1"=="" goto :done_parsing
if "%1"=="-t" (
    set "BUILD_TYPE=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="--type" (
    set "BUILD_TYPE=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="-d" (
    set "BUILD_DIR=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="--dir" (
    set "BUILD_DIR=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="-j" (
    set "JOBS=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="--jobs" (
    set "JOBS=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="-c" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if "%1"=="--clean" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if "%1"=="-v" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
if "%1"=="--verbose" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
if "%1"=="--cuda" (
    set "USE_CUDA=ON"
    shift
    goto :parse_args
)
if "%1"=="--vulkan" (
    set "USE_VULKAN=ON"
    shift
    goto :parse_args
)
if "%1"=="--metal" (
    set "USE_METAL=ON"
    shift
    goto :parse_args
)
if "%1"=="--native" (
    set "ENABLE_NATIVE_OPT=ON"
    shift
    goto :parse_args
)
if "%1"=="-h" goto :print_usage
if "%1"=="--help" goto :print_usage

echo Error: Unknown option %1
goto :print_usage

:print_usage
echo Kolosal Agent Build Script for Windows
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   -t, --type TYPE       Build type (Debug, Release, RelWithDebInfo) [default: Release]
echo   -d, --dir DIR         Build directory [default: build]
echo   -j, --jobs N          Number of parallel jobs [default: %NUMBER_OF_PROCESSORS%]
echo   -c, --clean           Clean build (remove build directory first)
echo   -v, --verbose         Verbose build output
echo   --cuda                Enable CUDA support
echo   --vulkan              Enable Vulkan support  
echo   --native              Enable native CPU optimization
echo   -h, --help            Show this help message
echo.
echo Examples:
echo   %0                    # Release build
echo   %0 -t Debug          # Debug build
echo   %0 -c                # Clean release build
echo   %0 --cuda            # Release build with CUDA support
echo   %0 -t Debug -v       # Verbose debug build
goto :eof

:done_parsing

REM Validate build type
if "%BUILD_TYPE%"=="Debug" goto :valid_build_type
if "%BUILD_TYPE%"=="Release" goto :valid_build_type
if "%BUILD_TYPE%"=="RelWithDebInfo" goto :valid_build_type
if "%BUILD_TYPE%"=="MinSizeRel" goto :valid_build_type

echo Error: Invalid build type '%BUILD_TYPE%'
echo Valid types: Debug, Release, RelWithDebInfo, MinSizeRel
exit /b 1

:valid_build_type

echo === Kolosal Agent Build Script ===
echo Project Root: %PROJECT_ROOT%
echo Build Type: %BUILD_TYPE%
echo Build Directory: %BUILD_DIR%
echo Parallel Jobs: %JOBS%
echo Clean Build: %CLEAN_BUILD%
echo Verbose: %VERBOSE%
echo CUDA Support: %USE_CUDA%
echo Vulkan Support: %USE_VULKAN%
echo Metal Support: %USE_METAL%
echo Native Optimization: %ENABLE_NATIVE_OPT%
echo ==================================

REM Check if we're in the right directory
if not exist "%PROJECT_ROOT%\CMakeLists.txt" (
    echo Error: CMakeLists.txt not found in %PROJECT_ROOT%
    echo Please run this script from the project root directory
    exit /b 1
)

REM Check for git submodules
echo Checking git submodules...
if not exist "%PROJECT_ROOT%\kolosal-server\CMakeLists.txt" (
    echo Git submodules not found. Initializing...
    cd /d "%PROJECT_ROOT%"
    git submodule update --init --recursive
    if !errorlevel! neq 0 (
        echo Error: Failed to initialize git submodules
        exit /b 1
    )
    echo Git submodules initialized successfully
) else if not exist "%PROJECT_ROOT%\external\yaml-cpp\CMakeLists.txt" (
    echo Git submodules not found. Initializing...
    cd /d "%PROJECT_ROOT%"
    git submodule update --init --recursive
    if !errorlevel! neq 0 (
        echo Error: Failed to initialize git submodules
        exit /b 1
    )
    echo Git submodules initialized successfully
) else (
    echo Git submodules already initialized
)

REM Clean build if requested
if "%CLEAN_BUILD%"=="true" (
    if exist "%PROJECT_ROOT%\%BUILD_DIR%" (
        echo Removing existing build directory...
        rmdir /s /q "%PROJECT_ROOT%\%BUILD_DIR%"
    )
)

REM Create build directory
if not exist "%PROJECT_ROOT%\%BUILD_DIR%" (
    mkdir "%PROJECT_ROOT%\%BUILD_DIR%"
)
cd /d "%PROJECT_ROOT%\%BUILD_DIR%"

REM Detect Visual Studio or build tools
set "GENERATOR="
set "PLATFORM="

REM Check for Visual Studio 2022
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "GENERATOR=Visual Studio 17 2022"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "GENERATOR=Visual Studio 17 2022"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "GENERATOR=Visual Studio 17 2022"
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "GENERATOR=Visual Studio 16 2019"
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "GENERATOR=Visual Studio 16 2019"
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "GENERATOR=Visual Studio 16 2019"
) else (
    echo Warning: Visual Studio not found, using default generator
)

REM Set platform architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set "PLATFORM=x64"
) else (
    set "PLATFORM=Win32"
)

REM Prepare cmake arguments
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%
set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_CUDA=%USE_CUDA%
set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_VULKAN=%USE_VULKAN%
set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_METAL=%USE_METAL%
set CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_NATIVE_OPTIMIZATION=%ENABLE_NATIVE_OPT%

if not "%GENERATOR%"=="" (
    set CMAKE_ARGS=%CMAKE_ARGS% -G "%GENERATOR%"
    set CMAKE_ARGS=%CMAKE_ARGS% -A %PLATFORM%
)

REM Run cmake configuration
echo.
echo Configuring build with CMake...
echo CMake command: cmake %CMAKE_ARGS% ..
cmake %CMAKE_ARGS% ..

if %errorlevel% neq 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

REM Build the project
echo.
echo Building project...
set BUILD_ARGS=--build . --config %BUILD_TYPE%

if "%VERBOSE%"=="true" (
    set BUILD_ARGS=%BUILD_ARGS% --verbose
)

if not "%JOBS%"=="" (
    set BUILD_ARGS=%BUILD_ARGS% --parallel %JOBS%
)

echo Build command: cmake %BUILD_ARGS%
cmake %BUILD_ARGS%

if %errorlevel% neq 0 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo === Build Successful ===
echo Executables built:
if exist "%BUILD_TYPE%\kolosal-agent.exe" (
    dir "%BUILD_TYPE%\kolosal-agent.exe"
)
if exist "kolosal-agent.exe" (
    dir "kolosal-agent.exe"
)
if exist "%BUILD_TYPE%\kolosal-launcher.exe" (
    dir "%BUILD_TYPE%\kolosal-launcher.exe"
)
if exist "kolosal-launcher.exe" (
    dir "kolosal-launcher.exe"
)
if exist "%BUILD_TYPE%\kolosal-server.exe" (
    dir "%BUILD_TYPE%\kolosal-server.exe"
)
if exist "kolosal-server.exe" (
    dir "kolosal-server.exe"
)
echo.
echo To run the agent system:
echo   cd %BUILD_DIR%
if exist "%BUILD_TYPE%\kolosal-agent.exe" (
    echo   %BUILD_TYPE%\kolosal-agent.exe --help
) else (
    echo   kolosal-agent.exe --help
)
echo.
echo Build completed successfully!

endlocal
