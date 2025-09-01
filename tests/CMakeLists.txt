# Tests CMakeLists.txt for Kolosal Agent

# Override runtime library for tests to use dynamic runtime to avoid conflicts
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
endif()

# GoogleTest is included via external/CMakeLists.txt

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann)
include_directories(${CMAKE_SOURCE_DIR}/external/yaml-cpp/include)

# Find required packages
find_package(Threads REQUIRED)

# Create a static library for core components to avoid linking issues
set(CORE_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/agent.cpp
    ${CMAKE_SOURCE_DIR}/src/core/agent_config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/agent_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/model_interface.cpp
    ${CMAKE_SOURCE_DIR}/src/core/kolosal_client.cpp
    ${CMAKE_SOURCE_DIR}/src/core/kolosal_server_launcher.cpp
    ${CMAKE_SOURCE_DIR}/src/workflows/workflow_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/workflows/workflow_types.cpp
    ${CMAKE_SOURCE_DIR}/src/api/http_server.cpp
)

# Add retrieval sources if enabled
if(BUILD_WITH_RETRIEVAL)
    list(APPEND CORE_TEST_SOURCES
        ${CMAKE_SOURCE_DIR}/src/core/retrieval_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/functions/retrieval_functions.cpp
        ${CMAKE_SOURCE_DIR}/src/functions/deep_research_functions.cpp
    )
endif()

# Create static library for core components
add_library(kolosal_core_test STATIC ${CORE_TEST_SOURCES})
target_include_directories(kolosal_core_test PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(kolosal_core_test PRIVATE yaml-cpp)

# Set dynamic runtime library for the core test library
if(MSVC)
    set_target_properties(kolosal_core_test PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# Enable BUILD_WITH_RETRIEVAL for test builds
if(BUILD_WITH_RETRIEVAL)
    target_compile_definitions(kolosal_core_test PRIVATE BUILD_WITH_RETRIEVAL)
endif()

# Always enable debug build and extensive logging for test components
target_compile_definitions(kolosal_core_test PRIVATE 
    DEBUG_BUILD
    KOLOSAL_TEST_BUILD
    EXTENSIVE_DEBUG_LOG
)

# Define test sources for unit tests
if(BUILD_TESTS)
    # Include GoogleTest for unit tests
    include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googletest/include)
    include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googlemock/include)
endif()

# Define test sources for unit tests
if(BUILD_TESTS)
    # Include GoogleTest for unit tests
    include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googletest/include)
    include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googlemock/include)

    set(TEST_SOURCES
        test_main.cpp
        test_agent.cpp
        test_agent_config.cpp
        test_agent_manager.cpp
        test_logger.cpp
        test_model_interface.cpp
        test_workflow_manager.cpp
        test_workflow_types.cpp
        test_http_server.cpp
        test_kolosal_client.cpp
        test_kolosal_server_launcher.cpp
    )

    # Add retrieval tests if enabled
    if(BUILD_WITH_RETRIEVAL)
        list(APPEND TEST_SOURCES
            test_retrieval_manager.cpp
            test_retrieval_functions.cpp
            test_deep_research_functions.cpp
        )
    endif()

    # Create test executable
    add_executable(kolosal_agent_tests ${TEST_SOURCES})

    # Set dynamic runtime library for the test executable
    if(MSVC)
        set_target_properties(kolosal_agent_tests PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        )
    endif()

    # Link libraries
    target_link_libraries(kolosal_agent_tests
        kolosal_core_test
        gtest
        gtest_main
        gmock
        gmock_main
        Threads::Threads
        yaml-cpp
    )

    # Set compile definitions
    if(BUILD_WITH_RETRIEVAL)
        target_compile_definitions(kolosal_agent_tests PRIVATE BUILD_WITH_RETRIEVAL)
    endif()

    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(kolosal_agent_tests ws2_32 winhttp rpcrt4)
    else()
        target_link_libraries(kolosal_agent_tests curl uuid)
    endif()

    # Add tests to CTest
    add_test(NAME KolosalAgentTests COMMAND kolosal_agent_tests)

    # Set test properties
    set_tests_properties(KolosalAgentTests PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        TIMEOUT 300
    )

    # Create individual test targets for specific test suites
    add_test(NAME AgentTests COMMAND kolosal_agent_tests --gtest_filter="AgentTest.*")
    add_test(NAME AgentConfigTests COMMAND kolosal_agent_tests --gtest_filter="AgentConfigTest.*")
    add_test(NAME AgentManagerTests COMMAND kolosal_agent_tests --gtest_filter="AgentManagerTest.*")
    add_test(NAME LoggerTests COMMAND kolosal_agent_tests --gtest_filter="LoggerTest.*")
    add_test(NAME ModelInterfaceTests COMMAND kolosal_agent_tests --gtest_filter="ModelInterfaceTest.*")
    add_test(NAME WorkflowManagerTests COMMAND kolosal_agent_tests --gtest_filter="WorkflowManagerTest.*")
    add_test(NAME WorkflowTypesTests COMMAND kolosal_agent_tests --gtest_filter="WorkflowTypesTest.*")
    add_test(NAME HttpServerTests COMMAND kolosal_agent_tests --gtest_filter="HttpServerTest.*")
    add_test(NAME KolosalClientTests COMMAND kolosal_agent_tests --gtest_filter="KolosalClientTest.*")
    add_test(NAME KolosalServerLauncherTests COMMAND kolosal_agent_tests --gtest_filter="KolosalServerLauncherTest.*")

    if(BUILD_WITH_RETRIEVAL)
        add_test(NAME RetrievalManagerTests COMMAND kolosal_agent_tests --gtest_filter="RetrievalManagerTest.*")
        add_test(NAME RetrievalFunctionsTests COMMAND kolosal_agent_tests --gtest_filter="RetrievalFunctionsTest.*")
        add_test(NAME DeepResearchFunctionsTests COMMAND kolosal_agent_tests --gtest_filter="DeepResearchFunctionsTest.*")
    endif()

endif()

# ==============================================================================
# Kolosal Agent Test Executable (kolosal-agent-test.exe)
# ==============================================================================

# Build test executable if enabled
if(BUILD_TEST_EXECUTABLE OR BUILD_TESTS)
    
    # Include GoogleTest for the test executable
    include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googletest/include)
    include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googlemock/include)
    
    # Test executable main source (comprehensive test runner)
    set(TEST_EXECUTABLE_MAIN_SOURCE test_main_integration_simple.cpp)
    
    # Test sources for the comprehensive test executable
    set(TEST_EXECUTABLE_SOURCES
        test_agent.cpp
        test_agent_config.cpp
        test_agent_manager.cpp
        test_logger.cpp
        test_model_interface.cpp
        test_workflow_manager.cpp
        test_workflow_types.cpp
        test_http_server.cpp
        test_kolosal_client.cpp
        test_kolosal_server_launcher.cpp
        test_integration.cpp
    )

    # Add retrieval tests if enabled
    if(BUILD_WITH_RETRIEVAL)
        list(APPEND TEST_EXECUTABLE_SOURCES
            test_retrieval_manager.cpp
            test_retrieval_functions.cpp
            test_deep_research_functions.cpp
        )
    endif()
    
    # Create test_main_executable.cpp if it doesn't exist
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_main_executable.cpp")
        message(STATUS "Creating test_main_executable.cpp for kolosal-agent-test.exe")
        
        set(TEST_MAIN_CONTENT "#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <thread>
#include \"logger.hpp\"
#include \"agent_config.hpp\"
#include \"agent_manager.hpp\"
#include \"workflow_manager.hpp\"

using namespace KolosalAgent;

class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Configure extensive debug logging
        auto& logger = Logger::instance();
        logger.set_level(LogLevel::DEBUG_LVL);
        logger.set_console_output(true);
        logger.set_file_output(\"kolosal_agent_test_debug.log\");
        logger.enable_timestamps(true);
        logger.enable_thread_id(true);
        logger.enable_function_tracing(true);
        
        LOG_INFO(\"=== Kolosal Agent Test Suite Starting ===\");
        LOG_INFO(\"Test Environment Setup Complete\");
        LOG_DEBUG(\"Debug logging enabled for comprehensive testing\");
        
        // Initialize system components
        try {
            LOG_INFO(\"Initializing test environment components...\");
            
            // Test configuration loading
            LOG_DEBUG(\"Testing configuration system...\");
            
            // Test basic agent manager
            LOG_DEBUG(\"Testing agent manager initialization...\");
            
            LOG_INFO(\"Test environment initialization complete\");
        } catch (const std::exception& e) {
            LOG_ERROR(\"Failed to initialize test environment: %s\", e.what());
            throw;
        }
    }
    
    void TearDown() override {
        LOG_INFO(\"=== Kolosal Agent Test Suite Completed ===\");
        LOG_INFO(\"Test Environment Cleanup Complete\");
    }
};

// Custom test listener for detailed logging
class DetailedTestListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        LOG_INFO(\"Starting Test: %s.%s\", test_info.test_case_name(), test_info.name());
        test_start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - test_start_time_);
        
        if (test_info.result()->Passed()) {
            LOG_INFO(\"Test PASSED: %s.%s (Duration: %lldms)\", 
                    test_info.test_case_name(), test_info.name(), duration.count());
        } else {
            LOG_ERROR(\"Test FAILED: %s.%s (Duration: %lldms)\", 
                     test_info.test_case_name(), test_info.name(), duration.count());
        }
    }
    
    void OnTestCaseStart(const ::testing::TestCase& test_case) override {
        LOG_INFO(\"=== Starting Test Case: %s ===\", test_case.name());
    }
    
    void OnTestCaseEnd(const ::testing::TestCase& test_case) override {
        LOG_INFO(\"=== Completed Test Case: %s (Tests: %d, Failures: %d) ===\", 
                test_case.name(), test_case.total_test_count(), test_case.failed_test_count());
    }

private:
    std::chrono::high_resolution_clock::time_point test_start_time_;
};

int main(int argc, char **argv) {
    std::cout << \"Kolosal Agent Test Executable (kolosal-agent-test.exe)\" << std::endl;
    std::cout << \"======================================================\" << std::endl;
    std::cout << \"Build Configuration: Debug with Extensive Logging\" << std::endl;
    std::cout << \"Test Framework: Google Test\" << std::endl;
    std::cout << \"======================================================\" << std::endl;
    
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add custom environment
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    
    // Add detailed test listener
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new DetailedTestListener);
    
    // Configure test output
    ::testing::GTEST_FLAG(print_time) = true;
    ::testing::GTEST_FLAG(color) = \"yes\";
    
    std::cout << \"Starting comprehensive test suite...\" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << \"======================================================\" << std::endl;
    std::cout << \"Test execution completed with result: \" << result << std::endl;
    std::cout << \"Check kolosal_agent_test_debug.log for detailed logs\" << std::endl;
    std::cout << \"======================================================\" << std::endl;
    
    return result;
}
")
        
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/test_main_executable.cpp" "${TEST_MAIN_CONTENT}")
    endif()
    
    # Create the comprehensive test executable
    add_executable(kolosal-agent-test 
        ${TEST_EXECUTABLE_MAIN_SOURCE}
        ${TEST_EXECUTABLE_SOURCES}
    )
    
    # Set output name
    set_target_properties(kolosal-agent-test PROPERTIES
        OUTPUT_NAME "kolosal-agent-test"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
    )
    
    # Set dynamic runtime library for the test executable
    if(MSVC)
        set_target_properties(kolosal-agent-test PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        )
    endif()
    
    # Enable extensive debug logging and test-specific definitions
    target_compile_definitions(kolosal-agent-test PRIVATE 
        DEBUG_BUILD
        KOLOSAL_TEST_BUILD
        EXTENSIVE_DEBUG_LOG
    )
    
    # Set compile definitions
    if(BUILD_WITH_RETRIEVAL)
        target_compile_definitions(kolosal-agent-test PRIVATE BUILD_WITH_RETRIEVAL)
    endif()
    
    # Link libraries
    target_link_libraries(kolosal-agent-test
        kolosal_core_test
        gtest
        gtest_main
        gmock
        gmock_main
        Threads::Threads
        yaml-cpp
    )
    
    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(kolosal-agent-test ws2_32 winhttp rpcrt4 winmm bcrypt)
        
        # Ensure kolosal-agent-test.exe is also copied to the main build directory
        add_custom_command(TARGET kolosal-agent-test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:kolosal-agent-test>"
            "${CMAKE_BINARY_DIR}/kolosal-agent-test.exe"
            COMMENT "Copying kolosal-agent-test.exe to main build directory"
        )
        
    elseif(APPLE)
        target_link_libraries(kolosal-agent-test
            "-framework Foundation"
            "-framework CoreFoundation"
            "-framework Security"
            pthread
            dl
        )
    else()
        target_link_libraries(kolosal-agent-test pthread dl rt)
        
        # Find and link UUID library if available
        find_library(UUID_LIBRARY uuid)
        if(UUID_LIBRARY)
            target_link_libraries(kolosal-agent-test ${UUID_LIBRARY})
        endif()
    endif()
    
    # Create an alias target for convenience
    add_custom_target(kolosal-agent-test-system ALL DEPENDS kolosal-agent-test)
    
    message(STATUS "Kolosal Agent Test Executable (kolosal-agent-test.exe) will be built")
    message(STATUS "  - Extensive debug logging: ENABLED")
    message(STATUS "  - Function tracing: ENABLED")
    message(STATUS "  - Performance timing: ENABLED")
    message(STATUS "  - Console output: ENABLED")
    message(STATUS "  - File logging: kolosal_agent_test_debug.log")
    
endif()