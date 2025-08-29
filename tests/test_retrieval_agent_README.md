# Retrieval Agent Test Documentation

## Overview

The `test_retrieval_agent.cpp` file contains comprehensive tests for the Kolosal Agent retrieval system functionality. This test suite validates both mock and real implementations of the retrieval manager.

## Test Categories

### 1. MockRetrievalManager Tests
The test suite includes a complete mock implementation of the retrieval manager that simulates:

- **Document Operations**
  - Adding documents with title and content
  - Searching documents by query with text matching
  - Listing documents with pagination support
  - Removing documents by ID
  
- **Internet Search** (when enabled)
  - Mock web search results
  - Search parameter validation
  - Result formatting

- **Combined Search**
  - Combines local document search with web search
  - Handles partial failures gracefully
  - Configurable search options

### 2. Test Suite Components

#### RetrievalManager Basic Operations
- Tests manager initialization and availability
- Validates status reporting
- Checks initial state (empty document store)

#### Document Operations
- **Adding Documents**: Tests document creation with title and content
- **Listing Documents**: Verifies document retrieval with pagination
- **Removing Documents**: Tests document deletion by ID
- **Document Count**: Validates document counting functionality

#### Document Search
- **Text Search**: Tests query-based document retrieval
- **Result Limiting**: Validates search result pagination
- **No Results**: Tests behavior when no matches found
- **Result Scoring**: Verifies mock similarity scoring

#### Internet Search
- **Disabled Search**: Tests behavior when internet search is disabled
- **Enabled Search**: Tests mock web search functionality
- **Result Formatting**: Validates web search result structure

#### Combined Search
- **Local + Web**: Tests combined local and internet search
- **Partial Failures**: Handles failures in individual search components
- **Configuration Dependent**: Respects search enable/disable settings

#### Error Scenarios
- **Unavailable Manager**: Tests behavior when retrieval system is unavailable
- **Invalid Parameters**: Tests validation of empty or missing parameters
- **Duplicate Operations**: Tests handling of invalid operations

#### Configuration Testing
- **Default Values**: Validates default configuration parameters
- **Custom Configuration**: Tests custom configuration acceptance
- **Parameter Validation**: Ensures configuration parameters are properly handled

### 3. Real RetrievalManager Integration

The test suite also attempts to test the real `RetrievalManager` implementation:

- **Conditional Testing**: Only runs if the retrieval system is available
- **Status Validation**: Checks real system status reporting
- **Error Handling**: Gracefully handles unavailable systems
- **Build Configuration**: Respects `BUILD_WITH_RETRIEVAL` flag

### 4. Agent Integration (Conditional)

When `BUILD_WITH_RETRIEVAL` is defined, additional tests cover:

- **Agent Function Registration**: Tests retrieval function availability in agents
- **Function Execution**: Tests calling retrieval functions through agent interface
- **Configuration Integration**: Tests agent-level retrieval configuration

## Running the Tests

### Individual Test Execution
```bash
# Run just the retrieval agent test
cd tests_build
.\Debug\test_retrieval_agent.exe
```

### CTest Integration
```bash
# Run with CTest
cd tests_build
ctest -C Debug -R RetrievalAgentTests --verbose
```

### Full Test Suite
```bash
# Run all tests including retrieval
cd tests_build
ctest -C Debug --verbose
```

## Test Results

The test suite provides detailed output showing:

- ✓ Passed assertions with descriptive messages
- Test category summaries
- Real system availability status
- Conditional test execution notifications
- Overall pass/fail summary

Expected output for successful run:
```
=== Kolosal Agent Retrieval System Tests ===
✓ RetrievalManager Basic PASSED
✓ Document Operations PASSED
✓ Document Search PASSED
✓ Internet Search PASSED
✓ Combined Search PASSED
✓ Error Scenarios PASSED
✓ Configuration Scenarios PASSED
✓ Real RetrievalManager PASSED
ℹ Agent with Retrieval test skipped (BUILD_WITH_RETRIEVAL not defined)

=== Test Summary ===
Passed: 8/8
🎉 All tests passed!
```

## Mock Implementation Features

The `MockRetrievalManager` class provides:

- **In-Memory Document Store**: Simulates document persistence
- **Text-Based Search**: Simple substring matching for queries
- **Pagination Support**: Implements offset/limit parameters
- **Error Simulation**: Can simulate unavailable states
- **Configuration Flexibility**: Supports various configuration scenarios

## Integration with Build System

The test is fully integrated with the CMake build system:

- **Automatic Discovery**: CMake automatically detects and builds the test
- **Dependency Management**: Properly links with required libraries
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **CI/CD Ready**: Suitable for automated testing pipelines

## Future Enhancements

Potential improvements for the test suite:

1. **Performance Testing**: Add benchmarks for large document sets
2. **Concurrency Testing**: Test thread-safety of retrieval operations
3. **Error Recovery**: Test system recovery from various failure modes
4. **Integration Testing**: Test with real vector databases (FAISS, Qdrant)
5. **Search Quality**: Add tests for search relevance and ranking

## Dependencies

The test relies on:

- **nlohmann/json**: For JSON parsing and manipulation
- **C++17 Standard**: For modern C++ features
- **CMake**: For build system integration
- **Optional: Kolosal Server**: For real retrieval system testing

## Notes

- The test is designed to work even when the full retrieval system is not available
- Mock implementation provides comprehensive testing coverage
- Real system integration is optional and gracefully degraded
- All tests include comprehensive error handling and validation
