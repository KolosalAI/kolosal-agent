# CMake Configuration Options

This document describes all available CMake configuration options for the Kolosal Agent System.

## Quick Reference

### Test Mode (Recommended for Development)
```bash
cmake .. -DENABLE_TEST_MODE=ON
```

### Production Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Full Development Build
```bash
cmake .. -DENABLE_TEST_MODE=ON -DENABLE_MEMORY_TESTING=ON -DENABLE_TEST_COVERAGE=ON -DBUILD_EXAMPLES=ON
```

## Core Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `CMAKE_BUILD_TYPE` | STRING | Debug | Build configuration: Debug, Release, RelWithDebInfo, MinSizeRel |
| `BUILD_SHARED_LIBS` | BOOL | OFF | Build shared libraries instead of static |

## Feature Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `BUILD_TESTS` | BOOL | OFF | Build unit tests |
| `BUILD_EXAMPLES` | BOOL | OFF | Build example applications |
| `BUILD_DOCS` | BOOL | OFF | Build documentation |
| `ENABLE_LOGGING` | BOOL | ON | Enable logging support |
| `ENABLE_METRICS` | BOOL | ON | Enable metrics collection |
| `ENABLE_HEALTH_MONITORING` | BOOL | ON | Enable health monitoring |
| `USE_SYSTEM_LIBS` | BOOL | OFF | Use system-installed libraries when possible |
| `ENABLE_HTTP_CLIENT` | BOOL | ON | Enable HTTP client functionality (requires CURL) |

## Test Mode Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `ENABLE_TEST_MODE` | BOOL | OFF | **Enable comprehensive test mode with enhanced debugging** |
| `BUILD_UNIT_TESTS` | BOOL | OFF | Build unit tests (included in BUILD_TESTS) |
| `BUILD_INTEGRATION_TESTS` | BOOL | OFF | Build integration tests (included in BUILD_TESTS) |
| `BUILD_PERFORMANCE_TESTS` | BOOL | OFF | Build performance tests (included in BUILD_TESTS) |
| `ENABLE_TEST_COVERAGE` | BOOL | OFF | Enable test coverage reporting (requires GCC/Clang) |
| `ENABLE_MEMORY_TESTING` | BOOL | OFF | Enable memory testing with sanitizers |
| `RUN_TESTS_ON_BUILD` | BOOL | OFF | Automatically run tests after successful build |
| `ENABLE_BENCHMARK_TESTS` | BOOL | OFF | Enable benchmark testing with Google Benchmark |

### Test Mode Behavior

When `ENABLE_TEST_MODE=ON` is set, the following happens automatically:
- Sets `BUILD_TESTS=ON`
- Sets `BUILD_UNIT_TESTS=ON`
- Sets `BUILD_INTEGRATION_TESTS=ON` 
- Sets `BUILD_PERFORMANCE_TESTS=ON`
- Sets `RUN_TESTS_ON_BUILD=ON`
- Sets `ENABLE_LOGGING=ON`
- Sets `ENABLE_METRICS=ON`
- Sets `VERBOSE_BUILD=ON`
- Forces Debug build type if Release was selected
- If `ENABLE_MEMORY_TESTING=ON`, also enables `ENABLE_ASAN=ON` and `ENABLE_UBSAN=ON`

## Advanced Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `USE_PODOFO` | BOOL | OFF | Enable PDF processing with PoDoFo |
| `ENABLE_CUDA` | BOOL | OFF | Enable CUDA support for GPU acceleration |
| `ENABLE_VULKAN` | BOOL | OFF | Enable Vulkan support |
| `ENABLE_METAL` | BOOL | OFF | Enable Metal support (macOS) |
| `ENABLE_NATIVE_OPTS` | BOOL | OFF | Enable native CPU optimizations |
| `MCP_PROTOCOL_ENABLED` | BOOL | ON | Enable Model Context Protocol integration |

## Development Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `ENABLE_ASAN` | BOOL | OFF | Enable AddressSanitizer |
| `ENABLE_TSAN` | BOOL | OFF | Enable ThreadSanitizer |
| `ENABLE_UBSAN` | BOOL | OFF | Enable UndefinedBehaviorSanitizer |

## Build Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `CLEAN_BUILD` | BOOL | OFF | Perform clean build |
| `INSTALL_AFTER_BUILD` | BOOL | OFF | Install after build |
| `PACKAGE_AFTER_BUILD` | BOOL | OFF | Create packages after build |
| `RUN_TESTS_AFTER_BUILD` | BOOL | OFF | Run tests after build (legacy) |
| `VERBOSE_BUILD` | BOOL | OFF | Verbose build output |
| `INIT_SUBMODULES` | BOOL | ON | Initialize git submodules on configure/build |

## Common Configurations

### Development with Full Testing
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TEST_MODE=ON \
  -DENABLE_MEMORY_TESTING=ON \
  -DENABLE_TEST_COVERAGE=ON \
  -DENABLE_BENCHMARK_TESTS=ON \
  -DBUILD_EXAMPLES=ON
```

### CI/CD Build
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DENABLE_TEST_COVERAGE=ON \
  -DRUN_TESTS_AFTER_BUILD=ON
```

### Minimal Build (Fastest)
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_DOCS=OFF
```

### Performance Analysis
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DENABLE_BENCHMARK_TESTS=ON \
  -DBUILD_PERFORMANCE_TESTS=ON \
  -DENABLE_NATIVE_OPTS=ON
```

### Memory Debugging
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_ASAN=ON \
  -DENABLE_UBSAN=ON \
  -DBUILD_TESTS=ON
```

## Dependencies

Different options require different dependencies:

### Required Always
- CMake 3.14+
- C++17 compiler
- Git

### For Tests (BUILD_TESTS=ON)
- Google Test (GTest)
- Google Mock (GMock)

### For Coverage (ENABLE_TEST_COVERAGE=ON)
- GCC or Clang compiler
- lcov (for HTML reports)

### For Benchmarks (ENABLE_BENCHMARK_TESTS=ON)
- Google Benchmark

### For HTTP Client (ENABLE_HTTP_CLIENT=ON)
- libcurl

### For PDF Support (USE_PODOFO=ON)
- PoDoFo library

### For GPU Acceleration
- CUDA Toolkit (ENABLE_CUDA=ON)
- Vulkan SDK (ENABLE_VULKAN=ON)

## Troubleshooting

### Tests Not Building
```bash
# Install dependencies first
vcpkg install gtest gmock  # Windows
# or
sudo apt install libgtest-dev libgmock-dev  # Ubuntu
```

### Coverage Not Working
- Ensure using GCC or Clang
- Install lcov: `sudo apt install lcov`

### Memory Testing Issues  
- Memory testing requires GCC or Clang
- MSVC doesn't support AddressSanitizer

### Performance Issues
- Use Release build for production
- Enable native optimizations: `-DENABLE_NATIVE_OPTS=ON`
- Consider GPU acceleration options

## Validation

To verify your configuration:

```bash
# Check configuration
cmake .. [your options]

# Validate test environment (if test mode enabled)
cmake --build . --target validate_test_environment --config Debug

# Show all available targets
cmake --build . --target help

# Show system information
cmake --build . --target info
```
