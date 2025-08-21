# CMakeLists.txt for Kolosal Agent Tests
cmake_minimum_required(VERSION 3.14)

# Enable testing
enable_testing()

# GoogleTest is configured from external/googletest directory by parent CMakeLists.txt
# The parent CMakeLists.txt handles GoogleTest setup when BUILD_TESTS=ON

# Optional: Find Google Benchmark if benchmark tests are enabled
if(ENABLE_BENCHMARK_TESTS)
    find_package(benchmark QUIET)
    if(benchmark_FOUND)
        message(STATUS "Google Benchmark found - benchmark tests will be built")
        set(BENCHMARK_AVAILABLE TRUE)
    else()
        message(WARNING "Google Benchmark not found - benchmark tests will be disabled")
        set(BENCHMARK_AVAILABLE FALSE)
    endif()
else()
    set(BENCHMARK_AVAILABLE FALSE)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kolosal-server/include
    ${CMAKE_SOURCE_DIR}/external/nlohmann
    ${CMAKE_SOURCE_DIR}/external/yaml-cpp/include
    ${CMAKE_SOURCE_DIR}/external/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/external/googletest/googlemock/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)

# Test configuration definitions
add_definitions(-DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
add_definitions(-DTEST_OUTPUT_DIR="${CMAKE_BINARY_DIR}/test_output")
add_definitions(-DTEST_LOGS_DIR="${CMAKE_BINARY_DIR}/test_logs")
add_definitions(-DTEST_CONFIGS_DIR="${CMAKE_BINARY_DIR}/test_configs")

# Test mode specific definitions
if(ENABLE_TEST_MODE)
    add_definitions(-DKOLOSAL_AGENT_TEST_MODE)
    add_definitions(-DKOLOSAL_AGENT_DEBUG_VERBOSE)
    message(STATUS "Test mode definitions added")
endif()

# Common test utilities library
add_library(test_utils STATIC
    mocks/mock_agent_components.cpp
    mocks/mock_llm_service.cpp
    mocks/mock_filesystem.cpp
    mocks/mock_deep_research_components.hpp
)

target_link_libraries(test_utils
    PUBLIC
        GTest::gtest
        GTest::gmock
        kolosal_agent
        Threads::Threads
)

target_include_directories(test_utils
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/kolosal-server/include
        ${CMAKE_SOURCE_DIR}/external/nlohmann
        ${CMAKE_SOURCE_DIR}/external/yaml-cpp/include
        ${CMAKE_SOURCE_DIR}/tests/mocks
)

# Configure test coverage
if(ENABLE_TEST_COVERAGE AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_compile_options(test_utils PRIVATE --coverage)
    target_link_options(test_utils PRIVATE --coverage)
    message(STATUS "Test coverage enabled for test_utils")
endif()

# Unit Tests
if(BUILD_UNIT_TESTS)
    set(UNIT_TEST_SOURCES
        # Agent tests - main versions with working content
        unit/agent/test_agent_core.cpp
        unit/agent/test_agent_interfaces.cpp
        
        # Function execution tests - enhanced with basic functionality
        unit/execution/test_function_execution_manager.cpp
        unit/execution/test_task_job_manager.cpp
        
        # Workflow tests - main version with working content
        unit/workflow/test_workflow_engine.cpp
        
        # Examples tests - Deep Research Agent tests
        unit/examples/test_deep_research_agent.cpp
        
        # Placeholder tests for all remaining categories
        unit/placeholder_tests.cpp
    )

    # Check which test files actually exist
    set(EXISTING_UNIT_TEST_SOURCES)
    foreach(source ${UNIT_TEST_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            list(APPEND EXISTING_UNIT_TEST_SOURCES ${source})
        else()
            message(STATUS "Test file not found, skipping: ${source}")
        endif()
    endforeach()

    # Only create executable if we have existing sources
    if(EXISTING_UNIT_TEST_SOURCES)
        # Create unit test executable
        add_executable(kolosal_agent_unit_tests ${EXISTING_UNIT_TEST_SOURCES})

        target_link_libraries(kolosal_agent_unit_tests
            PRIVATE
                test_utils
                GTest::gtest_main
                GTest::gmock_main
                kolosal_agent
                Threads::Threads
        )

        # Set test properties
        set_target_properties(kolosal_agent_unit_tests PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )

        # Configure test coverage for unit tests
        if(ENABLE_TEST_COVERAGE AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
            target_compile_options(kolosal_agent_unit_tests PRIVATE --coverage)
            target_link_options(kolosal_agent_unit_tests PRIVATE --coverage)
        endif()

        # Copy required DLLs for Windows
        if(WIN32)
            add_custom_command(TARGET kolosal_agent_unit_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/kolosal-server/bin/${CMAKE_BUILD_TYPE}/libcurl.dll
                    $<TARGET_FILE_DIR:kolosal_agent_unit_tests>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/kolosal-server/bin/${CMAKE_BUILD_TYPE}/llama-cpu.dll
                    $<TARGET_FILE_DIR:kolosal_agent_unit_tests>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/kolosal_server.dll
                    $<TARGET_FILE_DIR:kolosal_agent_unit_tests>
                COMMENT "Copying required DLLs for unit tests"
                VERBATIM
            )
        endif()

        message(STATUS "Unit tests configured with ${EXISTING_UNIT_TEST_SOURCES}")
    else()
        message(WARNING "No unit test sources found - unit tests disabled")
    endif()
endif()

# Integration Tests
if(BUILD_INTEGRATION_TESTS)
    set(INTEGRATION_TEST_SOURCES
        # Integration tests for Deep Research Agent
        integration/examples/test_deep_research_agent_integration.cpp
        
        # Integration tests are included in placeholder_tests.cpp for now
        # This ensures all integration test categories are represented
        # but using working placeholder implementations
    )

    # Check which integration test files actually exist
    set(EXISTING_INTEGRATION_TEST_SOURCES)
    foreach(source ${INTEGRATION_TEST_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            list(APPEND EXISTING_INTEGRATION_TEST_SOURCES ${source})
        else()
            message(STATUS "Integration test file not found, skipping: ${source}")
        endif()
    endforeach()

    # Only create executable if we have existing sources
    if(EXISTING_INTEGRATION_TEST_SOURCES)
        add_executable(kolosal_agent_integration_tests ${EXISTING_INTEGRATION_TEST_SOURCES})

        target_link_libraries(kolosal_agent_integration_tests
            PRIVATE
                test_utils
                GTest::gtest_main
                GTest::gmock_main
                kolosal_agent
                Threads::Threads
        )

        set_target_properties(kolosal_agent_integration_tests PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )

        # Configure test coverage for integration tests
        if(ENABLE_TEST_COVERAGE AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
            target_compile_options(kolosal_agent_integration_tests PRIVATE --coverage)
            target_link_options(kolosal_agent_integration_tests PRIVATE --coverage)
        endif()

        # Copy required DLLs for Windows
        if(WIN32)
            add_custom_command(TARGET kolosal_agent_integration_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/kolosal-server/bin/${CMAKE_BUILD_TYPE}/libcurl.dll
                    $<TARGET_FILE_DIR:kolosal_agent_integration_tests>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/kolosal-server/bin/${CMAKE_BUILD_TYPE}/llama-cpu.dll
                    $<TARGET_FILE_DIR:kolosal_agent_integration_tests>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/kolosal_server.dll
                    $<TARGET_FILE_DIR:kolosal_agent_integration_tests>
                COMMENT "Copying required DLLs for integration tests"
                VERBATIM
            )
        endif()

        message(STATUS "Integration tests configured with ${EXISTING_INTEGRATION_TEST_SOURCES}")
    else()
        message(WARNING "No integration test sources found - integration tests disabled")
    endif()
endif()

# Performance Tests
if(BUILD_PERFORMANCE_TESTS)
    set(PERFORMANCE_TEST_SOURCES
        # Performance tests for Deep Research Agent
        performance/examples/test_deep_research_agent_performance.cpp
        
        # Performance tests are included in placeholder_tests.cpp for now
        # This ensures all performance test categories are represented
        # but using working placeholder implementations
    )

    # Check which performance test files actually exist
    set(EXISTING_PERFORMANCE_TEST_SOURCES)
    foreach(source ${PERFORMANCE_TEST_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            list(APPEND EXISTING_PERFORMANCE_TEST_SOURCES ${source})
        else()
            message(STATUS "Performance test file not found, skipping: ${source}")
        endif()
    endforeach()

    # Only create executable if we have existing sources
    if(EXISTING_PERFORMANCE_TEST_SOURCES)
        add_executable(kolosal_agent_performance_tests ${EXISTING_PERFORMANCE_TEST_SOURCES})

        target_link_libraries(kolosal_agent_performance_tests
            PRIVATE
                test_utils
                GTest::gtest_main
                GTest::gmock_main
                kolosal_agent
                Threads::Threads
        )

        set_target_properties(kolosal_agent_performance_tests PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )

        # Copy required DLLs for Windows
        if(WIN32)
            add_custom_command(TARGET kolosal_agent_performance_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/kolosal-server/bin/${CMAKE_BUILD_TYPE}/libcurl.dll
                    $<TARGET_FILE_DIR:kolosal_agent_performance_tests>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/kolosal-server/bin/${CMAKE_BUILD_TYPE}/llama-cpu.dll
                    $<TARGET_FILE_DIR:kolosal_agent_performance_tests>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/kolosal_server.dll
                    $<TARGET_FILE_DIR:kolosal_agent_performance_tests>
                COMMENT "Copying required DLLs for performance tests"
                VERBATIM
            )
        endif()

        message(STATUS "Performance tests configured with ${EXISTING_PERFORMANCE_TEST_SOURCES}")
    else()
        message(WARNING "No performance test sources found - performance tests disabled")
    endif()
endif()

# Benchmark Tests (if Google Benchmark is available)
if(ENABLE_BENCHMARK_TESTS)
    set(BENCHMARK_TEST_SOURCES
        benchmarks/benchmark_agent_operations.cpp
        benchmarks/benchmark_workflow_execution.cpp
        benchmarks/benchmark_memory_operations.cpp
        benchmarks/benchmark_api_performance.cpp
    )

    # Check which benchmark test files actually exist
    set(EXISTING_BENCHMARK_TEST_SOURCES)
    foreach(source ${BENCHMARK_TEST_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            list(APPEND EXISTING_BENCHMARK_TEST_SOURCES ${source})
        else()
            message(STATUS "Benchmark test file not found, skipping: ${source}")
        endif()
    endforeach()

    # Only create executable if we have existing sources
    if(EXISTING_BENCHMARK_TEST_SOURCES)
        add_executable(kolosal_agent_benchmark_tests ${EXISTING_BENCHMARK_TEST_SOURCES})

        # Configure target based on whether Google Benchmark is available
        if(BENCHMARK_AVAILABLE)
            target_link_libraries(kolosal_agent_benchmark_tests
                PRIVATE
                    test_utils
                    benchmark::benchmark
                    kolosal_agent
                    Threads::Threads
            )
            
            # Define that benchmark library is found
            target_compile_definitions(kolosal_agent_benchmark_tests PRIVATE BENCHMARK_FOUND)
            
            message(STATUS "Benchmark tests configured with Google Benchmark")
        else()
            target_link_libraries(kolosal_agent_benchmark_tests
                PRIVATE
                    kolosal_agent
                    Threads::Threads
            )
            
            message(STATUS "Benchmark tests configured without Google Benchmark (fallback mode)")
        endif()

        set_target_properties(kolosal_agent_benchmark_tests PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )
    else()
        message(WARNING "No benchmark test sources found - benchmark tests disabled")
    endif()
endif()

# Register tests with CTest
include(GoogleTest)

# Register unit tests
if(BUILD_UNIT_TESTS AND TARGET kolosal_agent_unit_tests)
    gtest_discover_tests(kolosal_agent_unit_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PROPERTIES 
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            LABELS "unit"
            TIMEOUT 300
    )
endif()

# Register integration tests  
if(BUILD_INTEGRATION_TESTS AND TARGET kolosal_agent_integration_tests)
    gtest_discover_tests(kolosal_agent_integration_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PROPERTIES 
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            LABELS "integration"
            TIMEOUT 600
    )
endif()

# Register performance tests
if(BUILD_PERFORMANCE_TESTS AND TARGET kolosal_agent_performance_tests)
    gtest_discover_tests(kolosal_agent_performance_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PROPERTIES 
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            LABELS "performance"
            TIMEOUT 1200
    )
endif()

# Register benchmark tests
if(ENABLE_BENCHMARK_TESTS AND TARGET kolosal_agent_benchmark_tests)
    # Benchmark tests are registered differently
    add_test(NAME kolosal_agent_benchmarks
        COMMAND kolosal_agent_benchmark_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(kolosal_agent_benchmarks PROPERTIES 
        LABELS "benchmark"
        TIMEOUT 1800
    )
endif()

# Create test output directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_output)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_logs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_configs)

# Custom test targets
if(BUILD_UNIT_TESTS)
    add_custom_target(run_unit_tests
        COMMAND kolosal_agent_unit_tests
        DEPENDS kolosal_agent_unit_tests
        COMMENT "Running unit tests"
    )
endif()

if(BUILD_INTEGRATION_TESTS)
    add_custom_target(run_integration_tests
        COMMAND kolosal_agent_integration_tests
        DEPENDS kolosal_agent_integration_tests
        COMMENT "Running integration tests"
    )
endif()

if(BUILD_PERFORMANCE_TESTS)
    add_custom_target(run_performance_tests
        COMMAND kolosal_agent_performance_tests
        DEPENDS kolosal_agent_performance_tests
        COMMENT "Running performance tests"
    )
endif()

if(ENABLE_BENCHMARK_TESTS AND TARGET kolosal_agent_benchmark_tests)
    add_custom_target(run_benchmark_tests
        COMMAND kolosal_agent_benchmark_tests
        DEPENDS kolosal_agent_benchmark_tests
        COMMENT "Running benchmark tests"
    )
endif()

# Master test target that runs all configured tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
    DEPENDS 
        $<$<BOOL:${BUILD_UNIT_TESTS}>:kolosal_agent_unit_tests>
        $<$<BOOL:${BUILD_INTEGRATION_TESTS}>:kolosal_agent_integration_tests>
        $<$<BOOL:${BUILD_PERFORMANCE_TESTS}>:kolosal_agent_performance_tests>
        $<$<BOOL:${ENABLE_BENCHMARK_TESTS}>:kolosal_agent_benchmark_tests>
    COMMENT "Running all configured tests"
)

# Test mode specific targets
if(ENABLE_TEST_MODE)
    # Validation target to check test environment
    add_custom_target(validate_test_environment
        COMMAND ${CMAKE_COMMAND} -E echo "Validating test environment..."
        COMMAND ${CMAKE_COMMAND} -E echo "Test data dir: ${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
        COMMAND ${CMAKE_COMMAND} -E echo "Test output dir: ${CMAKE_BINARY_DIR}/test_output"
        COMMAND ${CMAKE_COMMAND} -E echo "Test logs dir: ${CMAKE_BINARY_DIR}/test_logs"
        COMMAND ${CMAKE_COMMAND} -E echo "Test configs dir: ${CMAKE_BINARY_DIR}/test_configs"
        COMMAND ${CMAKE_COMMAND} -E echo "Test environment validation complete"
        COMMENT "Validating test environment setup"
    )

    # Memory testing target (if memory testing is enabled)
    if(ENABLE_MEMORY_TESTING)
        add_custom_target(run_memory_tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L "unit|integration"
            COMMENT "Running tests with memory sanitizers enabled"
        )
    endif()
endif()

# Test coverage (if available)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(ENABLE_COVERAGE "Enable test coverage reporting" OFF)
    if(ENABLE_COVERAGE)
        target_compile_options(kolosal_agent_unit_tests PRIVATE --coverage)
        target_compile_options(kolosal_agent_integration_tests PRIVATE --coverage)
        target_link_options(kolosal_agent_unit_tests PRIVATE --coverage)
        target_link_options(kolosal_agent_integration_tests PRIVATE --coverage)
        
        add_custom_target(coverage
            COMMAND lcov --directory . --capture --output-file coverage.info
            COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND lcov --list coverage.info
            COMMAND genhtml coverage.info --output-directory coverage_report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating test coverage report"
        )
    endif()
endif()
