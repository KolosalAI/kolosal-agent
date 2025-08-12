# Kolosal Agent System - Test Mode Guide

## Overview

The Kolosal Agent System now includes a comprehensive **Test Mode** that provides enhanced testing capabilities, debugging features, and performance analysis tools. Test mode is designed to help developers ensure code quality, identify performance bottlenecks, and validate system behavior.

## Features

### 🧪 Test Mode Features

- **Comprehensive Test Coverage**: Unit, Integration, Performance, and Benchmark tests
- **Memory Testing**: AddressSanitizer and UndefinedBehaviorSanitizer integration
- **Test Coverage Reports**: Line and branch coverage analysis (GCC/Clang)
- **Benchmark Testing**: Google Benchmark integration for performance analysis
- **Automated Test Execution**: Tests run automatically after successful builds
- **Enhanced Debugging**: Verbose logging and debug symbols
- **Test Environment Validation**: Automated setup verification

### 📊 Test Types

1. **Unit Tests** (`BUILD_UNIT_TESTS`)
   - Individual component testing
   - Mocked dependencies
   - Fast execution (< 5 minutes)
   - Label: `unit`

2. **Integration Tests** (`BUILD_INTEGRATION_TESTS`)
   - Component interaction testing
   - Real dependencies where possible
   - Medium execution time (5-15 minutes)
   - Label: `integration`

3. **Performance Tests** (`BUILD_PERFORMANCE_TESTS`)
   - Performance regression detection
   - Memory usage validation
   - Longer execution time (15-30 minutes)
   - Label: `performance`

4. **Benchmark Tests** (`ENABLE_BENCHMARK_TESTS`)
   - Detailed performance analysis
   - Comparative benchmarking
   - Statistical analysis
   - Label: `benchmark`

## Quick Start

### Enable Test Mode

```bash
# Enable comprehensive test mode
cmake .. -DENABLE_TEST_MODE=ON

# Build everything including tests
cmake --build . --config Debug

# Tests run automatically, or run manually:
cmake --build . --target full-test --config Debug
```

### Advanced Test Mode Options

```bash
# Full test mode with all features
cmake .. \
  -DENABLE_TEST_MODE=ON \
  -DENABLE_MEMORY_TESTING=ON \
  -DENABLE_TEST_COVERAGE=ON \
  -DENABLE_BENCHMARK_TESTS=ON

# Build and run
cmake --build . --config Debug
```

### Selective Test Building

```bash
# Enable only specific test types
cmake .. \
  -DBUILD_TESTS=ON \
  -DBUILD_UNIT_TESTS=ON \
  -DBUILD_INTEGRATION_TESTS=OFF \
  -DBUILD_PERFORMANCE_TESTS=ON

cmake --build . --config Debug
```

## Test Commands

### Basic Test Commands

```bash
# Run all tests
ctest --output-on-failure

# Run specific test categories
ctest -L unit                    # Unit tests only
ctest -L integration             # Integration tests only  
ctest -L performance             # Performance tests only
ctest -L benchmark               # Benchmark tests only

# Run tests in parallel
ctest --parallel 4
```

### Test Mode Specific Targets

```bash
# Build for test mode
cmake --build . --target build-test-mode --config Debug

# Quick unit tests (fast feedback)
cmake --build . --target quick-test --config Debug

# Full test suite
cmake --build . --target full-test --config Debug

# Test with detailed reporting
cmake --build . --target test-report --config Debug
```

### Memory Testing

```bash
# Enable memory testing
cmake .. -DENABLE_TEST_MODE=ON -DENABLE_MEMORY_TESTING=ON
cmake --build . --config Debug

# Run memory tests
cmake --build . --target memory_tests --config Debug
```

### Coverage Reporting

```bash
# Enable coverage (GCC/Clang only)
cmake .. -DENABLE_TEST_MODE=ON -DENABLE_TEST_COVERAGE=ON
cmake --build . --config Debug

# Generate coverage report
cmake --build . --target test_coverage --config Debug

# View coverage report
open coverage_report/index.html  # macOS
start coverage_report/index.html # Windows
```

## Directory Structure

```
tests/
├── CMakeLists.txt              # Test configuration
├── README.md                   # This file
├── fixtures/                   # Test data and configurations
│   ├── test_config.yaml       # Test configuration file
│   └── test_fixtures.hpp      # Test fixture definitions
├── mocks/                      # Mock implementations
│   ├── mock_agent_components.cpp
│   ├── mock_llm_service.cpp
│   └── mock_filesystem.cpp
├── unit/                       # Unit tests
│   ├── agent/                  # Agent component tests
│   ├── api/                    # API tests
│   ├── config/                 # Configuration tests
│   └── ...
├── integration/                # Integration tests
│   ├── test_full_system_integration.cpp
│   ├── test_server_integration.cpp
│   └── ...
├── performance/                # Performance tests
│   ├── test_agent_performance.cpp
│   ├── test_memory_performance.cpp
│   └── ...
└── benchmarks/                 # Benchmark tests (Google Benchmark)
    ├── benchmark_agent_operations.cpp
    ├── benchmark_workflow_execution.cpp
    └── ...
```

## Test Output

### Output Directories

- `build/test_output/` - Test result files
- `build/test_logs/` - Detailed test logs
- `build/test_configs/` - Generated test configurations
- `build/coverage_report/` - Coverage reports (if enabled)

### Test Reports

Test mode generates detailed reports including:
- Test execution summaries
- Performance metrics
- Memory usage statistics
- Coverage percentages
- Benchmark results

## Configuration Options

### Core Test Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_TEST_MODE` | OFF | Enable comprehensive test mode |
| `BUILD_TESTS` | OFF | Build test executables |
| `BUILD_UNIT_TESTS` | OFF | Build unit tests |
| `BUILD_INTEGRATION_TESTS` | OFF | Build integration tests |
| `BUILD_PERFORMANCE_TESTS` | OFF | Build performance tests |

### Advanced Test Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_TEST_COVERAGE` | OFF | Enable coverage reporting |
| `ENABLE_MEMORY_TESTING` | OFF | Enable sanitizers |
| `ENABLE_BENCHMARK_TESTS` | OFF | Enable benchmark tests |
| `RUN_TESTS_ON_BUILD` | OFF | Auto-run tests after build |

## Dependencies

### Required for Basic Testing
- Google Test (GTest)
- Google Mock (GMock)

### Optional for Advanced Features
- Google Benchmark (for benchmark tests)
- lcov/gcov (for coverage reports)
- GCC/Clang (for sanitizers and coverage)

### Installation

#### Windows (vcpkg)
```bash
vcpkg install gtest gmock benchmark
```

#### Ubuntu/Debian
```bash
sudo apt install libgtest-dev libgmock-dev libbenchmark-dev lcov
```

#### macOS (Homebrew)
```bash
brew install googletest google-benchmark lcov
```

## Continuous Integration

Test mode is designed to work seamlessly with CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Configure with Test Mode
  run: cmake -B build -DENABLE_TEST_MODE=ON -DENABLE_MEMORY_TESTING=ON

- name: Build with Tests
  run: cmake --build build --config Debug

- name: Run Tests
  run: cd build && ctest --output-on-failure --parallel 4
```

## Troubleshooting

### Common Issues

1. **Tests not building**
   - Ensure Google Test is installed
   - Check CMake finds the test dependencies

2. **Memory testing fails**
   - Memory testing requires GCC or Clang
   - MSVC doesn't support AddressSanitizer

3. **Coverage not generated**
   - Coverage requires GCC or Clang with `--coverage` support
   - Install `lcov` for HTML reports

4. **Benchmark tests missing**
   - Install Google Benchmark library
   - Enable with `-DENABLE_BENCHMARK_TESTS=ON`

### Debug Information

```bash
# Enable verbose build output
cmake .. -DENABLE_TEST_MODE=ON -DVERBOSE_BUILD=ON

# Check test environment
cmake --build . --target validate_test_environment --config Debug

# Verify test configuration
ctest --show-only
```

## Best Practices

1. **Run tests frequently** - Use `quick-test` for rapid feedback
2. **Use test mode in development** - Catches issues early
3. **Enable memory testing** - Detects memory leaks and corruption
4. **Monitor performance** - Use benchmark tests for optimization
5. **Maintain test coverage** - Aim for >80% coverage
6. **Write focused tests** - Keep unit tests fast and focused

## Performance Tips

- Use `-DCMAKE_BUILD_TYPE=Debug` for testing
- Enable parallel test execution: `ctest --parallel 4`
- Use labels to run specific test subsets
- Profile tests if they become too slow
- Consider test sharding for very large test suites

## Contributing

When adding new features:
1. Add corresponding unit tests
2. Update integration tests if needed
3. Add performance tests for critical paths
4. Document test-specific configuration
5. Ensure tests pass in CI environment

For more information, see the main project documentation.
