# Kolosal Agent System Test Suite

This directory contains comprehensive tests for the Kolosal Agent System v2.0, covering all major components and functionality.

## Quick Start (Minimal Test Demo)

For a quick demonstration that works without any external dependencies:

### Option 1: Direct Compilation
```powershell
# Compile and run the minimal test demo
cd tests
g++ -std=c++17 -o minimal_test_demo.exe minimal_test_demo.cpp
.\minimal_test_demo.exe
```

### Option 2: Using CMake Build System
```powershell
# Build with CMake (builds minimal_test_demo.exe successfully)
cd tests
.\run_tests.ps1 -TestType demo
# Then run directly:
cd tests_build\Debug
.\minimal_test_demo.exe
```

This will run a comprehensive test suite that demonstrates:
- **Agent Creation & Lifecycle**: Creating, starting, stopping agents  
- **Agent Capabilities**: Adding and managing agent capabilities
- **Agent Execution**: Task execution with proper state management
- **Agent Manager**: Multi-agent management, creation, removal, and search
- **Performance Metrics**: Creating 100 agents in milliseconds
- **Error Handling**: Graceful handling of invalid operations

**Test Results**: All 27 tests pass with 100% success rate!

The minimal test demo uses mock objects and doesn't require any external libraries or complex setup.

## Test Coverage

### 1. Agent Creation and Configuration Tests (`test_agent_execution.cpp`)
- **YAML Configuration Loading**: Tests loading and parsing of agent configuration files
- **Agent Creation with System Prompts**: Validates agent creation with custom system instructions
- **Configuration Validation**: Tests handling of invalid and partial configurations
- **Default Agent Initialization**: Tests automatic creation of agents from configuration

### 2. Agent Manager Functionality Tests (`test_agent_execution.cpp`)
- **Agent Lifecycle Management**: Create, start, stop, delete operations
- **Multi-Agent Management**: Concurrent agent operations and management
- **Agent State Tracking**: Running state, capabilities, and information retrieval
- **Configuration Integration**: Agent manager with configuration manager integration

### 3. Model Interface Integration Tests (`test_model_interface.cpp`)
- **Model Communication**: Basic model API communication
- **Parameter Validation**: Testing various parameter combinations and edge cases
- **Error Handling**: Graceful handling of model unavailability and timeouts
- **Fallback Scenarios**: Behavior when models are not available

### 4. HTTP API Endpoints Tests (`test_http_server.cpp`)
- **Server Lifecycle**: Start, stop, restart operations
- **Concurrent Requests**: Multiple simultaneous client handling
- **Resource Management**: Memory and connection cleanup
- **Port and Host Configuration**: Different binding configurations

### 5. Function Execution Tests (`test_agent_execution.cpp`)
- **Chat Function**: Message processing with and without model parameters
- **Analysis Function**: Text analysis with various parameters
- **Echo Function**: Simple data echoing for testing
- **Concurrent Execution**: Multiple simultaneous function calls
- **Timeout Handling**: Function execution time limits

### 6. Retrieval Agent System Tests (`test_retrieval_agent.cpp`)
- **Document Management**: Add, search, list, and remove documents in vector database
- **Internet Search**: Mock and real web search functionality
- **Combined Search**: Local document + internet search integration
- **Configuration Testing**: Retrieval system configuration validation
- **Error Scenarios**: Unavailable systems, invalid parameters, edge cases
- **Agent Integration**: Retrieval functions within agent execution context

### 7. Error Handling Tests (`test_error_scenarios.cpp`)
- **Configuration Errors**: Malformed YAML, missing files, invalid data types
- **Agent Creation Errors**: Invalid names, capabilities, excessive creation
- **Function Execution Errors**: Invalid functions, parameters, timeouts
- **Model Interface Errors**: Network timeouts, invalid URLs, extreme parameters
- **HTTP Server Errors**: Invalid ports, addresses, resource exhaustion
- **Data Corruption**: Invalid UTF-8, malformed JSON, large data handling

## Test Structure

### Test Files
- `test_agent_execution.cpp` - Main comprehensive test suite
- `test_model_interface.cpp` - Model interface specific tests
- `test_config_manager.cpp` - Configuration management tests
- `test_http_server.cpp` - HTTP server specific tests
- `test_retrieval_agent.cpp` - Retrieval system and document management tests
- `test_error_scenarios.cpp` - Error handling and edge cases

### Test Framework
- **Framework**: Google Test (GTest)
- **Timeout Support**: Tests include timeout handling for long-running operations
- **Async Testing**: Support for concurrent and asynchronous operations
- **Resource Cleanup**: Automatic cleanup of test artifacts and resources

### Test Categories

#### Quick Tests (~2 minutes)
- Model Interface Tests
- Configuration Manager Tests
- Basic unit tests with minimal setup

#### Integration Tests (~5 minutes)
- Agent Execution Tests
- HTTP Server Tests
- Full system integration scenarios

#### Stress Tests (~10 minutes)
- Error Scenario Tests
- Resource exhaustion testing
- High-load concurrent operations

## Running Tests

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler
- Google Test framework (included in project)
- Windows: Visual Studio Build Tools or full Visual Studio

### Quick Start
```powershell
# Run all tests
.\run_tests.ps1

# Run quick tests only
.\run_tests.ps1 -TestType quick

# Run with verbose output
.\run_tests.ps1 -TestType all -Verbose

# Save results to file
.\run_tests.ps1 -TestType integration -OutputFile results.json
```

### Manual Build and Run
```powershell
# Create build directory
mkdir tests_build
cd tests_build

# Configure with CMake
cmake -S ../tests -B . -DCMAKE_BUILD_TYPE=Debug

# Build tests
cmake --build . --config Debug

# Run individual tests
.\test_agent_execution.exe
.\test_model_interface.exe
.\test_config_manager.exe
.\test_http_server.exe
.\test_retrieval_agent.exe
.\test_error_scenarios.exe
```

### CMake Targets
```bash
# Build and run all tests
cmake --build . --target run_tests

# Run quick tests only
cmake --build . --target run_quick_tests

# Run integration tests
cmake --build . --target run_integration_tests

# Run stress tests
cmake --build . --target run_stress_tests
```

## Test Configuration

### Test Configuration Files
Tests automatically create temporary configuration files:
- `test_agent_config.yaml` - Main test configuration
- `test_model_config.yaml` - Model configuration for testing
- Various corrupted/invalid configs for error testing

### Environment Setup
- Tests use non-standard ports (8081-8103) to avoid conflicts
- Automatic cleanup of test artifacts
- Isolated test environments for each test case

### Timeout Configuration
- Function execution: 10-20 seconds
- Agent startup: 5 seconds
- Server operations: 2-5 seconds
- Stress tests: Up to 300 seconds

## Test Results and Reporting

### Output Formats
- **Console**: Colored output with progress indicators
- **XML**: Google Test XML format for CI/CD integration
- **JSON**: Custom detailed results format
- **Log Files**: Individual test execution logs

### Success Criteria
- **Unit Tests**: All assertions pass, no exceptions
- **Integration Tests**: Full workflow completion
- **Performance Tests**: Operations complete within time limits
- **Error Tests**: Graceful error handling, no crashes

### Metrics Tracked
- Test execution time
- Memory usage patterns
- Concurrent operation limits
- Error recovery effectiveness
- Resource cleanup completeness

## Continuous Integration

### GitHub Actions Integration
```yaml
- name: Run Agent Tests
  run: |
    cd tests
    .\run_tests.ps1 -TestType all -OutputFile test_results.json
```

### Test Coverage
- **Line Coverage**: Aimed at >80% of core functionality
- **Branch Coverage**: All major code paths tested
- **Error Paths**: Comprehensive error scenario coverage
- **Integration Coverage**: All public APIs tested

## Troubleshooting

### Common Issues
1. **Build Failures**: Check CMake version and compiler compatibility
2. **Test Timeouts**: Increase timeout values for slower systems
3. **Port Conflicts**: Tests use ports 8081-8103, ensure they're available
4. **Memory Issues**: Stress tests may require significant RAM

### Debug Mode
```powershell
# Run with maximum verbosity
.\run_tests.ps1 -TestType all -Verbose

# Run single test for debugging
.\tests_build\test_agent_execution.exe --gtest_filter="*SpecificTest*"
```

### Log Analysis
- Check `tests_build/test_results/*.log` for detailed execution logs
- XML output in `tests_build/test_results/*.xml` for CI parsing
- JSON results provide structured data for analysis

## Contributing

### Adding New Tests
1. Create test functions following existing patterns
2. Use appropriate test fixtures for setup/teardown
3. Include timeout handling for long operations
4. Add both positive and negative test cases
5. Update this README with new test descriptions

### Test Guidelines
- **Isolation**: Each test should be independent
- **Cleanup**: Always clean up resources in teardown
- **Assertions**: Use descriptive assertion messages
- **Coverage**: Test both success and failure paths
- **Performance**: Include timing checks for critical operations

## Future Enhancements

### Planned Improvements
- **Mock Interfaces**: Better mocking for external dependencies
- **Performance Benchmarks**: Standardized performance metrics
- **Load Testing**: Automated high-load scenario testing
- **Fuzzing**: Automated input fuzzing for robustness testing
- **Code Coverage**: Automated code coverage reporting
