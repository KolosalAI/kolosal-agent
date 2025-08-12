# Kolosal Agent Test Suite

This directory contains the comprehensive test suite for the Kolosal Agent System v2.0.

## Test Structure

The test suite is organized into three main categories:

### 1. Unit Tests (`tests/unit/`)
Individual component testing with isolated functionality verification:

- **Agent Core Tests** (`agent/`)
  - `test_agent_core.cpp` - Core agent functionality
  - `test_agent_interfaces.cpp` - Data structures and interfaces
  - `test_agent_roles.cpp` - Agent roles and specializations
  - `test_agent_factory.cpp` - Agent factory patterns
  - `test_multi_agent_system.cpp` - Multi-agent coordination
  - `test_agent_memory_manager.cpp` - Memory management
  - `test_agent_planning_system.cpp` - Planning and reasoning

- **Configuration Tests** (`config/`)
  - `test_yaml_configuration_parser.cpp` - YAML parsing and validation

- **Workflow Tests** (`workflow/`)
  - `test_workflow_engine.cpp` - Workflow execution engine
  - `test_sequential_workflow.cpp` - Sequential workflow implementation

- **API Tests** (`api/`)
  - `test_simple_http_server.cpp` - HTTP server functionality
  - `test_message_router.cpp` - Message routing
  - `test_http_client.cpp` - HTTP client operations

- **Additional Component Tests**
  - Execution, Tools, Server, Utils, Logger components

### 2. Integration Tests (`tests/integration/`)
End-to-end system testing with multiple components:

- `test_full_system_integration.cpp` - Complete system integration
- `test_multi_agent_workflows.cpp` - Multi-agent workflow scenarios
- `test_server_integration.cpp` - Server integration testing
- `test_configuration_loading.cpp` - Configuration system integration
- `test_mcp_integration.cpp` - Model Context Protocol integration
- `test_api_endpoints.cpp` - API endpoint integration

### 3. Performance Tests (`tests/performance/`)
Performance benchmarking and stress testing:

- `test_agent_performance.cpp` - Agent performance benchmarks
- `test_workflow_performance.cpp` - Workflow execution performance
- `test_memory_performance.cpp` - Memory operation performance
- `test_concurrent_execution.cpp` - Concurrency and threading tests

## Test Infrastructure

### Test Fixtures (`tests/fixtures/`)
- `test_fixtures.hpp/cpp` - Common test fixtures and utilities
- Provides base classes for different test scenarios
- Helper methods for test data creation

### Mock Objects (`tests/mocks/`)
- `mock_agent_components.hpp/cpp` - Mock agent components
- `mock_llm_service.hpp/cpp` - Mock LLM service for testing
- `mock_filesystem.hpp/cpp` - Mock filesystem operations

## Building and Running Tests

### Prerequisites
- Google Test (GTest) framework
- CMake 3.14+
- Same dependencies as main project

### Installation of GTest
```bash
# Using vcpkg (Windows)
vcpkg install gtest

# Using package manager (Ubuntu/Debian)
sudo apt install libgtest-dev

# Using Homebrew (macOS)
brew install googletest
```

### Build Configuration
```bash
# Configure with tests enabled
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# Build the project and tests
cmake --build build --config Debug

# Or build specific test targets
cmake --build build --target kolosal_agent_unit_tests
cmake --build build --target kolosal_agent_integration_tests
cmake --build build --target kolosal_agent_performance_tests
```

### Running Tests
```bash
# Run all tests using CTest
cd build
ctest --output-on-failure

# Run specific test categories
./kolosal_agent_unit_tests
./kolosal_agent_integration_tests
./kolosal_agent_performance_tests

# Run with GTest filters
./kolosal_agent_unit_tests --gtest_filter="AgentCoreTest.*"
./kolosal_agent_unit_tests --gtest_filter="*Performance*"
```

### Custom Test Targets
```bash
# Run specific test categories using custom targets
cmake --build build --target run_unit_tests
cmake --build build --target run_integration_tests
cmake --build build --target run_performance_tests
cmake --build build --target run_all_tests
```

## Test Coverage

To enable test coverage reporting (GCC/Clang):
```bash
cmake -B build -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
cmake --build build
cd build && ctest
cmake --build build --target coverage
```

Coverage reports will be generated in `build/coverage_report/`.

## Test Data

Test fixtures and data files are located in:
- `tests/fixtures/` - Test configuration files and data
- Runtime test output: `build/test_output/`

## Writing New Tests

### Adding Unit Tests
1. Create test file in appropriate `tests/unit/` subdirectory
2. Include necessary headers and test fixtures
3. Use Google Test macros (TEST, TEST_F, EXPECT_*, ASSERT_*)
4. Add the file to CMakeLists.txt UNIT_TEST_SOURCES

### Adding Integration Tests
1. Create test file in `tests/integration/`
2. Use comprehensive test fixtures that set up multiple components
3. Test realistic usage scenarios
4. Add the file to CMakeLists.txt INTEGRATION_TEST_SOURCES

### Adding Performance Tests
1. Create test file in `tests/performance/`
2. Use timing measurements and performance expectations
3. Include throughput and latency metrics
4. Add the file to CMakeLists.txt PERFORMANCE_TEST_SOURCES

### Example Test Structure
```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "component/header.hpp"
#include "../fixtures/test_fixtures.hpp"

using namespace testing;
using namespace kolosal::agents;
using namespace kolosal::agents::test;

class ComponentTest : public ComponentTestFixture {
protected:
    void SetUp() override {
        ComponentTestFixture::SetUp();
        // Component-specific setup
    }
    
    void TearDown() override {
        // Component-specific cleanup
        ComponentTestFixture::TearDown();
    }
};

TEST_F(ComponentTest, BasicFunctionality) {
    // Test basic functionality
    EXPECT_TRUE(component_->is_initialized());
    
    // Test operations
    auto result = component_->perform_operation();
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.data.empty());
}
```

## Best Practices

1. **Test Isolation**: Each test should be independent and not rely on other tests
2. **Descriptive Names**: Use clear, descriptive test names that explain what is being tested
3. **Setup/Teardown**: Use proper setup and teardown to ensure clean test environment
4. **Mock Objects**: Use mocks to isolate components and control dependencies
5. **Performance Tests**: Include realistic performance expectations and metrics
6. **Error Handling**: Test both success and error scenarios
7. **Concurrency**: Test thread safety and concurrent operations where applicable

## Continuous Integration

The test suite is designed to run in CI/CD environments:
- All tests should pass before merging
- Performance tests provide baseline metrics
- Integration tests verify system-level functionality
- Test results are collected and reported

## Troubleshooting

### Common Issues
1. **GTest Not Found**: Install Google Test framework
2. **Compilation Errors**: Ensure all dependencies are available
3. **Test Failures**: Check test logs for specific error messages
4. **Performance Issues**: Review system resources and test expectations

### Debug Mode
Run tests in debug mode for detailed information:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build
cd build && ctest --verbose
```

## Contributing

When contributing new features:
1. Add corresponding unit tests
2. Update integration tests if needed
3. Consider performance implications
4. Update this README if test structure changes

For more information about the Kolosal Agent System, see the main project documentation.
