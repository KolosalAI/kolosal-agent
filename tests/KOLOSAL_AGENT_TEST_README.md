# Kolosal Agent Test Executable

## Overview

The `kolosal-agent-test.exe` is a comprehensive test executable for the Kolosal Agent system. It provides extensive debugging capabilities and comprehensive testing of all system components.

## Features

### Extensive Debug Logging
- **Debug Level**: Set to `DEBUG_LVL` by default for maximum verbosity
- **Console Output**: Enabled for immediate feedback
- **File Output**: Logs to `kolosal_agent_test_debug.log` 
- **Timestamps**: All log entries include precise timestamps
- **Thread ID**: Multi-threaded logging with thread identification
- **Function Tracing**: Automatic entry/exit logging for functions
- **Performance Timing**: Built-in timing for performance analysis

### Test Coverage
- **Unit Tests**: All existing unit tests from the main test suite
- **Integration Tests**: Comprehensive system integration testing
- **Performance Tests**: Baseline performance measurements
- **Stress Tests**: Multi-threaded stress testing
- **Logger Functionality**: Comprehensive logging system testing

### Build Configuration
- **Debug Build**: Always built with `DEBUG_BUILD` enabled
- **Test-Specific Defines**: Additional test-specific compile definitions
- **Extensive Logging**: `EXTENSIVE_DEBUG_LOG` enabled by default
- **All Features**: Built with retrieval support and all available features

## Building

### Using CMake Directly
```bash
# Configure with test executable enabled
cmake -S . -B build -DBUILD_TEST_EXECUTABLE=ON -DCMAKE_BUILD_TYPE=Debug

# Build the test executable
cmake --build build --config Debug --target kolosal-agent-test
```

### Using VS Code Tasks
The following tasks are available in VS Code:
- **"CMake: Build Test Executable (kolosal-agent-test.exe)"** - Basic build
- **"CMake: Build Test Executable (Debug with extensive logging)"** - Full featured build
- **"Run kolosal-agent-test.exe"** - Build and run with standard options
- **"Run kolosal-agent-test.exe (verbose)"** - Build and run with verbose output

### Build Options
- `BUILD_TEST_EXECUTABLE=ON` - Enable test executable build
- `BUILD_KOLOSAL_SERVER=ON` - Include Kolosal Server components
- `BUILD_WITH_RETRIEVAL=ON` - Include retrieval functionality
- `CMAKE_BUILD_TYPE=Debug` - Debug build with extensive logging

## Running

### Basic Execution
```bash
# Run from build directory
./kolosal-agent-test.exe

# Or from project root
./build/kolosal-agent-test.exe
```

### Command Line Options
```bash
# Run with colored output and timing
./kolosal-agent-test.exe --gtest_color=yes --gtest_print_time=1

# Run specific test cases
./kolosal-agent-test.exe --gtest_filter="*Integration*"

# Run with verbose output
./kolosal-agent-test.exe --verbose

# List all available tests
./kolosal-agent-test.exe --gtest_list_tests
```

### Google Test Options
All standard Google Test command line options are supported:
- `--gtest_filter=PATTERN` - Run only tests matching pattern
- `--gtest_repeat=N` - Repeat tests N times
- `--gtest_shuffle` - Randomize test execution order
- `--gtest_break_on_failure` - Break on first failure
- `--gtest_color=yes|no|auto` - Control colored output

## Output

### Console Output
The test executable provides detailed console output including:
- Test execution progress with colored status indicators
- Individual test timing information
- Detailed test case summaries
- Overall execution statistics

### Log Files
- **`kolosal_agent_test_debug.log`** - Comprehensive debug log
- Includes all system operations with timestamps
- Function entry/exit tracing
- Performance measurements
- Error diagnostics

### Test Results
Example output:
```
Kolosal Agent Test Executable (kolosal-agent-test.exe)
======================================================
Build Configuration: Debug with Extensive Logging
Test Framework: Google Test
======================================================

=== Kolosal Agent Test Suite Starting ===
[INFO] Test Environment Setup Complete
[DEBUG] Debug logging enabled for comprehensive testing

Running comprehensive test suite...

[==========] Running N tests from M test cases.
[----------] Global test environment set-up.
[----------] X tests from KolosalAgentIntegrationTest
[ RUN      ] KolosalAgentIntegrationTest.BasicSystemInitialization
[INFO] Testing basic system initialization
[       OK ] KolosalAgentIntegrationTest.BasicSystemInitialization (Y ms)
...

======================================================
Test execution completed with result: 0
Check kolosal_agent_test_debug.log for detailed logs
======================================================
```

## Test Categories

### Unit Tests
- `test_agent.cpp` - Agent class functionality
- `test_agent_config.cpp` - Configuration management
- `test_agent_manager.cpp` - Agent lifecycle management
- `test_logger.cpp` - Logging system
- `test_model_interface.cpp` - Model interface
- `test_workflow_manager.cpp` - Workflow management
- `test_workflow_types.cpp` - Workflow type definitions
- `test_http_server.cpp` - HTTP server functionality
- `test_kolosal_client.cpp` - Kolosal client operations
- `test_kolosal_server_launcher.cpp` - Server launcher

### Integration Tests
- `test_integration.cpp` - Comprehensive system integration
  - Basic system initialization
  - Configuration loading
  - Agent manager initialization
  - Workflow manager initialization
  - Logger functionality testing
  - Performance baseline testing
  - Multi-threaded stress testing
  - Extensive debug logging validation

### Retrieval Tests (if enabled)
- `test_retrieval_manager.cpp` - Retrieval system management
- `test_retrieval_functions.cpp` - Retrieval functionality
- `test_deep_research_functions.cpp` - Research capabilities

## Debugging

### Debug Features
- **Function Tracing**: Use `TRACE_FUNCTION()` macro for automatic tracing
- **Scoped Timers**: Use `SCOPED_TIMER("name")` for performance measurement
- **Detailed Logging**: All levels available with context information
- **Thread Safety**: Multi-threaded logging with thread identification

### Debug Macros
```cpp
LOG_DEBUG("Debug message");
LOG_INFO("Info message");
LOG_WARN("Warning message");
LOG_ERROR("Error message");

LOG_DEBUG_F("Debug with format: %s %d", "test", 123);

TRACE_FUNCTION();  // Automatic function entry/exit logging
SCOPED_TIMER("operation_name");  // Automatic timing
```

### Performance Analysis
The test executable includes built-in performance measurement:
- Individual test timing
- Function execution timing
- System operation benchmarking
- Multi-threaded performance testing

## Troubleshooting

### Common Issues

**Build Fails**
- Ensure `BUILD_TEST_EXECUTABLE=ON` is set
- Check that GoogleTest is properly initialized in `external/`
- Verify all dependencies are available

**Runtime Failures**
- Check `kolosal_agent_test_debug.log` for detailed error information
- Ensure configuration files are accessible
- Verify model files are present if testing retrieval features

**Missing Test Output**
- Confirm test executable is built in the correct location
- Check console output and log file for error messages
- Ensure proper permissions for file creation

### Configuration
The test executable uses extensive debug configuration by default:
- Log level: `DEBUG_LVL`
- Console output: Enabled
- File output: `kolosal_agent_test_debug.log`
- All debug features: Enabled

## Development

### Adding New Tests
1. Create test files in the `tests/` directory
2. Add to `TEST_EXECUTABLE_SOURCES` in `CMakeLists.txt`
3. Follow Google Test conventions
4. Use extensive logging for debugging

### Modifying Test Configuration
Edit `test_main_executable.cpp` to modify:
- Logging configuration
- Test environment setup
- Custom test listeners
- Global test behavior

### Integration with CI/CD
The test executable can be integrated into continuous integration:
```bash
# Build test executable
cmake --build build --config Debug --target kolosal-agent-test

# Run tests with XML output
./build/kolosal-agent-test.exe --gtest_output=xml:test_results.xml
```

## License

This test executable is part of the Kolosal Agent project and follows the same licensing terms.
