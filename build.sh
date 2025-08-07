#!/bin/bash

# Kolosal Agent Build Script
# This script builds the Kolosal Agent system with the integrated server

set -e  # Exit on any error

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN_BUILD=false
VERBOSE=false
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
USE_CUDA=OFF
USE_VULKAN=OFF
USE_METAL=OFF
ENABLE_NATIVE_OPT=OFF

# Function to print usage
print_usage() {
    echo "Kolosal Agent Build Script"
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type TYPE       Build type (Debug, Release, RelWithDebInfo) [default: Release]"
    echo "  -d, --dir DIR         Build directory [default: build]" 
    echo "  -j, --jobs N          Number of parallel jobs [default: auto-detected]"
    echo "  -c, --clean           Clean build (remove build directory first)"
    echo "  -v, --verbose         Verbose build output"
    echo "  --cuda                Enable CUDA support"
    echo "  --vulkan              Enable Vulkan support"
    echo "  --metal               Enable Metal support (macOS only)"
    echo "  --native              Enable native CPU optimization (-march=native)"
    echo "  -h, --help            Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Release build"
    echo "  $0 -t Debug          # Debug build"
    echo "  $0 -c                # Clean release build"
    echo "  $0 --cuda            # Release build with CUDA support"
    echo "  $0 -t Debug -v       # Verbose debug build"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --cuda)
            USE_CUDA=ON
            shift
            ;;
        --vulkan)
            USE_VULKAN=ON
            shift
            ;;
        --metal)
            USE_METAL=ON
            shift
            ;;
        --native)
            ENABLE_NATIVE_OPT=ON
            shift
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo "Error: Unknown option $1"
            print_usage
            exit 1
            ;;
    esac
done

# Validate build type
case $BUILD_TYPE in
    Debug|Release|RelWithDebInfo|MinSizeRel)
        ;;
    *)
        echo "Error: Invalid build type '$BUILD_TYPE'"
        echo "Valid types: Debug, Release, RelWithDebInfo, MinSizeRel"
        exit 1
        ;;
esac

echo "=== Kolosal Agent Build Script ==="
echo "Project Root: $PROJECT_ROOT"
echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "Parallel Jobs: $JOBS"
echo "Clean Build: $CLEAN_BUILD"
echo "Verbose: $VERBOSE"
echo "CUDA Support: $USE_CUDA"
echo "Vulkan Support: $USE_VULKAN"
echo "Metal Support: $USE_METAL"
echo "Native Optimization: $ENABLE_NATIVE_OPT"
echo "=================================="

# Check if we're in the right directory
if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
    echo "Error: CMakeLists.txt not found in $PROJECT_ROOT"
    echo "Please run this script from the project root directory"
    exit 1
fi

# Check for git submodules
echo "Checking git submodules..."
if [[ ! -f "$PROJECT_ROOT/kolosal-server/CMakeLists.txt" ]] || [[ ! -f "$PROJECT_ROOT/external/yaml-cpp/CMakeLists.txt" ]]; then
    echo "Git submodules not found. Initializing..."
    cd "$PROJECT_ROOT"
    git submodule update --init --recursive
    if [[ $? -ne 0 ]]; then
        echo "Error: Failed to initialize git submodules"
        exit 1
    fi
    echo "Git submodules initialized successfully"
else
    echo "Git submodules already initialized"
fi

# Clean build if requested
if [[ "$CLEAN_BUILD" == true ]] && [[ -d "$PROJECT_ROOT/$BUILD_DIR" ]]; then
    echo "Removing existing build directory..."
    rm -rf "$PROJECT_ROOT/$BUILD_DIR"
fi

# Create build directory
mkdir -p "$PROJECT_ROOT/$BUILD_DIR"
cd "$PROJECT_ROOT/$BUILD_DIR"

# Prepare cmake arguments
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DUSE_CUDA="$USE_CUDA"
    -DUSE_VULKAN="$USE_VULKAN"
    -DUSE_METAL="$USE_METAL"
    -DENABLE_NATIVE_OPTIMIZATION="$ENABLE_NATIVE_OPT"
)

# Add platform-specific arguments
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS specific
    CMAKE_ARGS+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=10.14)
    if [[ "$USE_METAL" == "OFF" ]]; then
        # Enable Metal by default on macOS unless explicitly disabled
        CMAKE_ARGS[-4]=-DUSE_METAL=ON
        echo "Enabled Metal support by default on macOS"
    fi
fi

# Run cmake configuration
echo "Configuring build with CMake..."
echo "CMake command: cmake ${CMAKE_ARGS[*]} .."
cmake "${CMAKE_ARGS[@]}" ..

if [[ $? -ne 0 ]]; then
    echo "Error: CMake configuration failed"
    exit 1
fi

# Prepare make arguments
MAKE_ARGS=("-j$JOBS")
if [[ "$VERBOSE" == true ]]; then
    MAKE_ARGS+=("VERBOSE=1")
fi

# Build the project
echo ""
echo "Building project..."
echo "Make command: cmake --build . ${MAKE_ARGS[*]}"

if command -v ninja >/dev/null 2>&1 && [[ -f build.ninja ]]; then
    echo "Using Ninja build system"
    ninja "${MAKE_ARGS[@]}"
else
    echo "Using Make build system"
    cmake --build . -- "${MAKE_ARGS[@]}"
fi

if [[ $? -ne 0 ]]; then
    echo "Error: Build failed"
    exit 1
fi

echo ""
echo "=== Build Successful ==="
echo "Executables built:"
if [[ -f kolosal-agent* ]]; then
    ls -la kolosal-agent*
fi
if [[ -f kolosal-launcher* ]]; then
    ls -la kolosal-launcher*  
fi
if [[ -f kolosal-server* ]]; then
    ls -la kolosal-server*
fi
echo ""
echo "To run the agent system:"
echo "  cd $BUILD_DIR"
echo "  ./kolosal-agent --help"
echo ""
echo "Build completed successfully!"
