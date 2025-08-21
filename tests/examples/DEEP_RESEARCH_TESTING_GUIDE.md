# Deep Research Agent Testing Guide

This document provides comprehensive information about testing the Deep Research Agent implementation.

## Overview

The Deep Research Agent tests are organized into three main categories:

1. **Unit Tests** - Test individual components and methods in isolation
2. **Integration Tests** - Test the agent with server connectivity and real workflows
3. **Performance Tests** - Benchmark and stress-test the agent's performance

## Test Structure

```
tests/
├── unit/examples/
│   ├── test_deep_research_agent.cpp              # Basic unit tests
│   └── test_deep_research_agent_mocked.cpp       # Tests with mocked components
├── integration/examples/
│   └── test_deep_research_agent_integration.cpp  # End-to-end integration tests
├── performance/examples/
│   └── test_deep_research_agent_performance.cpp  # Performance benchmarks
├── mocks/
│   └── mock_deep_research_components.hpp         # Mock implementations
└── fixtures/
    └── deep_research_test_config.yaml            # Test configuration
```

## Unit Tests

### Basic Unit Tests (`test_deep_research_agent.cpp`)

Tests core functionality without external dependencies:

- **Constructor and Initialization**: Verify proper agent setup
- **Configuration Management**: Test configuration setting/getting
- **Basic Research**: Test research execution with fallback implementations
- **Workflow Management**: Test custom workflow creation and execution
- **Error Handling**: Test graceful error handling and edge cases

### Mocked Unit Tests (`test_deep_research_agent_mocked.cpp`)

Tests with comprehensive mocking for controlled testing:

- **Function Execution**: Test individual research phases with mocks
- **Parameter Validation**: Verify correct parameter passing
- **Error Scenarios**: Test failure handling with controlled failures
- **Concurrent Operations**: Test thread safety and concurrent execution

### Test Classes

- `DeepResearchAgentTest`: Basic functionality tests
- `DeepResearchAgentMockedTest`: Tests with mocked components
- `DeepResearchAgentFactoryTest`: Factory pattern tests
- `DeepResearchResultTest`: Research result structure tests

## Integration Tests

### Server Integration (`test_deep_research_agent_integration.cpp`)

Tests requiring live server connectivity:

- **Server Connectivity**: Test connection and function availability
- **End-to-End Research**: Complete research workflows with real server
- **Workflow Integration**: Test predefined and custom workflows
- **Data Quality**: Validate research content quality and relevance

### Running Integration Tests

Integration tests are disabled by default. To enable:

```bash
export KOLOSAL_INTEGRATION_TESTS=1
export KOLOSAL_SERVER_URL=http://localhost:8080  # Optional
```

## Performance Tests

### Performance Benchmarks (`test_deep_research_agent_performance.cpp`)

Comprehensive performance testing:

- **Initialization Performance**: Agent startup and initialization timing
- **Research Timing**: Basic research operation performance
- **Scalability**: Performance with varying source limits
- **Concurrency**: Multi-threaded research performance
- **Memory Usage**: Memory consumption patterns
- **Stress Testing**: Continuous operation under load

### Running Performance Tests

Performance tests are disabled by default. To enable:

```bash
export KOLOSAL_PERFORMANCE_TESTS=1
```

## Mock Components

### MockFunctionManager

Mocks the function execution system:

```cpp
auto mock_function_manager = std::make_shared<MockFunctionManager>();
mock_function_manager->setup_research_mocks();
```

### MockWorkflowExecutor

Mocks workflow execution:

```cpp
auto mock_workflow_executor = std::make_shared<MockWorkflowExecutor>();
mock_workflow_executor->setup_workflow_mocks();
```

### MockAgentCore

Mocks the core agent functionality:

```cpp
auto mock_agent_core = std::make_shared<MockAgentCore>();
mock_agent_core->setup_core_mocks(mock_function_manager);
```

## Test Configuration

### Configuration File

Tests use `tests/fixtures/deep_research_test_config.yaml` for configuration:

```yaml
test_environment:
  enable_integration_tests: false
  server_url: "http://localhost:8080"
  timeout_seconds: 300

research_test_config:
  default_methodology: "systematic"
  test_max_sources: 10
  test_confidence_threshold: 0.5
```

### Environment Variables

- `KOLOSAL_INTEGRATION_TESTS=1`: Enable integration tests
- `KOLOSAL_PERFORMANCE_TESTS=1`: Enable performance tests
- `KOLOSAL_SERVER_URL`: Override default server URL

## Running Tests

### Building Tests

```bash
cd build
cmake .. -DBUILD_TESTS=ON -DBUILD_UNIT_TESTS=ON -DBUILD_INTEGRATION_TESTS=ON -DBUILD_PERFORMANCE_TESTS=ON
cmake --build . --config Debug
```

### Running Unit Tests

```bash
# Run all unit tests
./kolosal_agent_unit_tests

# Run specific test suite
./kolosal_agent_unit_tests --gtest_filter="DeepResearchAgentTest.*"

# Run with verbose output
./kolosal_agent_unit_tests --gtest_filter="*" --gtest_output="xml:test_results.xml"
```

### Running Integration Tests

```bash
# Enable integration tests
export KOLOSAL_INTEGRATION_TESTS=1

# Run integration tests
./kolosal_agent_integration_tests

# Run specific integration test
./kolosal_agent_integration_tests --gtest_filter="*ServerConnectivity*"
```

### Running Performance Tests

```bash
# Enable performance tests
export KOLOSAL_PERFORMANCE_TESTS=1

# Run performance tests
./kolosal_agent_performance_tests

# Run specific performance test
./kolosal_agent_performance_tests --gtest_filter="*InitializationTime*"
```

### Running All Tests

```bash
# Using CTest
ctest --output-on-failure

# Using custom target
cmake --build . --target run_all_tests
```

## Test Coverage

### Enabling Coverage

```bash
cmake .. -DENABLE_TEST_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build .
./kolosal_agent_unit_tests
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

### Coverage Targets

- Unit tests should achieve >90% line coverage
- Integration tests should cover all major workflows
- Performance tests should validate all performance-critical paths

## Debugging Tests

### Debug Configuration

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TEST_MODE=ON
```

### Using Debugger

```bash
# GDB
gdb ./kolosal_agent_unit_tests
(gdb) run --gtest_filter="DeepResearchAgentTest.BasicResearchExecution"

# Visual Studio Code
# Set breakpoints in test files and launch debugger
```

### Verbose Logging

```cpp
// In test setup
ENABLE_VERBOSE_LOGGING=1
```

## Test Data and Fixtures

### Sample Data

Tests use predefined research questions and configurations from the test config file.

### Mock Responses

Mock functions provide realistic responses for testing without external dependencies.

### Test Environment Setup

Tests automatically create temporary directories and configuration files as needed.

## Common Issues and Solutions

### Server Connection Issues

**Problem**: Integration tests fail with connection errors
**Solution**: Ensure kolosal-server is running on the configured port

### Performance Test Timeouts

**Problem**: Performance tests exceed timeout limits
**Solution**: Adjust timeout values in test configuration or optimize test parameters

### Mock Setup Issues

**Problem**: Mock expectations not met
**Solution**: Verify mock setup in test fixtures and ensure proper call sequences

### Memory Leaks

**Problem**: Memory leaks detected in tests
**Solution**: Ensure proper cleanup in test teardown and check for circular references

## Best Practices

### Writing New Tests

1. Use descriptive test names that explain what is being tested
2. Follow the Arrange-Act-Assert pattern
3. Use mocks for external dependencies
4. Test both success and failure scenarios
5. Include edge cases and boundary conditions

### Test Organization

1. Group related tests in the same test class
2. Use test fixtures for common setup/teardown
3. Keep tests independent and isolated
4. Use appropriate test categories (unit/integration/performance)

### Performance Testing

1. Warm up before measuring performance
2. Run multiple iterations for stable measurements
3. Test with realistic data sizes
4. Monitor memory usage alongside timing

### Mock Usage

1. Use mocks for external dependencies
2. Set up realistic mock responses
3. Verify mock interactions when important
4. Don't over-mock internal functionality

## Continuous Integration

### CI Configuration

Tests are designed to run in CI environments with:

- Unit tests always enabled
- Integration tests disabled by default (require server)
- Performance tests disabled by default (time-consuming)

### Test Reporting

Tests generate XML output compatible with CI systems:

```bash
./kolosal_agent_unit_tests --gtest_output="xml:unit_test_results.xml"
```

## Extending Tests

### Adding New Test Cases

1. Identify the appropriate test category (unit/integration/performance)
2. Add test methods to existing test classes or create new ones
3. Update CMakeLists.txt if adding new test files
4. Add documentation for complex test scenarios

### Adding New Mock Components

1. Create mock class inheriting from the interface
2. Add to `mock_deep_research_components.hpp`
3. Update test utilities as needed
4. Document mock behavior and setup methods

This testing framework provides comprehensive coverage of the Deep Research Agent functionality, ensuring reliability, performance, and maintainability of the implementation.
