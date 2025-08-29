# CMakeLists.txt for Kolosal Agent System Tests
cmake_minimum_required(VERSION 3.16)

# Test project
project(KolosalAgentTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(../include)
include_directories(../external/nlohmann)
include_directories(../external/yaml-cpp/include)

# For Windows-specific socket functions
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# Try to find Google Test
find_path(GTEST_INCLUDE_DIR gtest/gtest.h 
    PATHS ../kolosal-server/external/yaml-cpp/test/gtest-1.11.0/googletest/include
    NO_DEFAULT_PATH
)

if(GTEST_INCLUDE_DIR)
    message(STATUS "Found Google Test at: ${GTEST_INCLUDE_DIR}")
    include_directories(${GTEST_INCLUDE_DIR})
    
    # Try to build gtest from source
    set(GTEST_SOURCE_DIR "${GTEST_INCLUDE_DIR}/../")
    if(EXISTS "${GTEST_SOURCE_DIR}/src/gtest-all.cc")
        add_library(gtest STATIC
            ${GTEST_SOURCE_DIR}/src/gtest-all.cc
            ${GTEST_SOURCE_DIR}/src/gtest_main.cc
        )
        target_include_directories(gtest PRIVATE ${GTEST_SOURCE_DIR})
        target_include_directories(gtest PUBLIC ${GTEST_INCLUDE_DIR})
    endif()
else()
    message(WARNING "Google Test not found - creating minimal test framework")
    # Create a minimal test framework if gtest is not available
endif()

# YAML-CPP setup
set(YAML_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/yaml-cpp")
if(EXISTS "${YAML_CPP_DIR}/CMakeLists.txt")
    add_subdirectory(${YAML_CPP_DIR} "${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp" EXCLUDE_FROM_ALL)
    message(STATUS "Using yaml-cpp from: ${YAML_CPP_DIR}")
else()
    message(FATAL_ERROR "yaml-cpp not found at ${YAML_CPP_DIR}")
endif()

# Source files for the main project (needed for testing)
set(KOLOSAL_AGENT_SOURCES
    ../src/core/agent.cpp
    ../src/core/agent_manager.cpp
    ../src/core/agent_config.cpp
    ../src/core/model_interface.cpp
    ../src/core/retrieval_manager.cpp
    ../src/api/http_server.cpp
    ../src/workflows/workflow_manager.cpp
    ../src/workflows/workflow_types.cpp
)

# Check which source files exist
set(AVAILABLE_SOURCES "")
foreach(source ${KOLOSAL_AGENT_SOURCES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
        list(APPEND AVAILABLE_SOURCES ${source})
        message(STATUS "Found source: ${source}")
    else()
        message(WARNING "Source file not found: ${source}")
    endif()
endforeach()

# Windows-specific libraries
if(WIN32)
    set(PLATFORM_LIBS ws2_32 wsock32)
else()
    set(PLATFORM_LIBS pthread)
endif()

# Helper function to create test executables
function(create_test_executable target_name source_file)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
        add_executable(${target_name} ${source_file} ${AVAILABLE_SOURCES})
        
        # Link libraries
        if(TARGET gtest)
            target_link_libraries(${target_name} gtest)
        endif()
        
        # Link yaml-cpp library
        target_link_libraries(${target_name} yaml-cpp)
        
        target_link_libraries(${target_name} ${PLATFORM_LIBS})
        
        # Set properties
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        message(STATUS "Created test executable: ${target_name}")
    else()
        message(WARNING "Test source not found: ${source_file}")
    endif()
endfunction()

# Create test executables
create_test_executable(test_model_interface test_model_interface.cpp)
create_test_executable(test_config_manager test_config_manager.cpp)
create_test_executable(test_workflow_config test_workflow_config.cpp)
create_test_executable(test_workflow_manager test_workflow_manager.cpp)
create_test_executable(test_workflow_orchestrator test_workflow_orchestrator.cpp)
create_test_executable(test_retrieval_agent test_retrieval_agent.cpp)
create_test_executable(simple_test_demo simple_test_demo.cpp)

# Add minimal test demo (standalone, no external dependencies)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/minimal_test_demo.cpp")
    add_executable(minimal_test_demo minimal_test_demo.cpp)
    set_target_properties(minimal_test_demo PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    message(STATUS "Created standalone test executable: minimal_test_demo")
endif()

# Only create complex tests if we have all dependencies
if(TARGET gtest AND AVAILABLE_SOURCES)
    create_test_executable(test_agent_execution test_agent_execution.cpp)
    create_test_executable(test_http_server test_http_server.cpp)
    create_test_executable(test_error_scenarios test_error_scenarios.cpp)
endif()

# Test discovery (if CTest is available)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20")
    enable_testing()
    
    if(TARGET simple_test_demo)
        add_test(NAME SimpleTestDemo COMMAND simple_test_demo)
        set_tests_properties(SimpleTestDemo PROPERTIES TIMEOUT 60)
    endif()
    
    if(TARGET minimal_test_demo)
        add_test(NAME MinimalTestDemo COMMAND minimal_test_demo)
        set_tests_properties(MinimalTestDemo PROPERTIES TIMEOUT 60)
    endif()
    
    if(TARGET test_model_interface)
        add_test(NAME ModelInterfaceTests COMMAND test_model_interface)
        set_tests_properties(ModelInterfaceTests PROPERTIES TIMEOUT 60)
    endif()
    
    if(TARGET test_config_manager)
        add_test(NAME ConfigManagerTests COMMAND test_config_manager)
        set_tests_properties(ConfigManagerTests PROPERTIES TIMEOUT 60)
    endif()
    
    if(TARGET test_retrieval_agent)
        add_test(NAME RetrievalAgentTests COMMAND test_retrieval_agent)
        set_tests_properties(RetrievalAgentTests PROPERTIES TIMEOUT 120)
    endif()
    
    if(TARGET test_workflow_config)
        add_test(NAME WorkflowConfigTests COMMAND test_workflow_config)
        set_tests_properties(WorkflowConfigTests PROPERTIES TIMEOUT 120)
    endif()
    
    if(TARGET test_workflow_manager)
        add_test(NAME WorkflowManagerTests COMMAND test_workflow_manager)
        set_tests_properties(WorkflowManagerTests PROPERTIES TIMEOUT 180)
    endif()
    
    if(TARGET test_workflow_orchestrator)
        add_test(NAME WorkflowOrchestratorTests COMMAND test_workflow_orchestrator)
        set_tests_properties(WorkflowOrchestratorTests PROPERTIES TIMEOUT 300)
    endif()
    
    if(TARGET test_agent_execution)
        add_test(NAME AgentExecutionTests COMMAND test_agent_execution)
        set_tests_properties(AgentExecutionTests PROPERTIES TIMEOUT 300)
    endif()
    
    if(TARGET test_http_server)
        add_test(NAME HTTPServerTests COMMAND test_http_server)
        set_tests_properties(HTTPServerTests PROPERTIES TIMEOUT 120)
    endif()
    
    if(TARGET test_error_scenarios)
        add_test(NAME ErrorScenarioTests COMMAND test_error_scenarios)
        set_tests_properties(ErrorScenarioTests PROPERTIES TIMEOUT 300)
    endif()
endif()

# Print build information
message(STATUS "Kolosal Agent Tests Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
if(TARGET gtest)
    message(STATUS "  Google Test: Available")
else()
    message(STATUS "  Google Test: Not available")
endif()
message(STATUS "  Available sources: ${AVAILABLE_SOURCES}")

# Custom targets for running tests
if(TARGET test_model_interface AND TARGET test_config_manager)
    add_custom_target(run_quick_tests
        COMMAND test_model_interface
        COMMAND test_config_manager
        COMMAND test_workflow_config
        COMMAND test_retrieval_agent
        DEPENDS test_model_interface test_config_manager test_workflow_config test_retrieval_agent
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

if(TARGET test_workflow_manager AND TARGET test_workflow_orchestrator)
    add_custom_target(run_workflow_tests
        COMMAND test_workflow_manager
        COMMAND test_workflow_orchestrator
        DEPENDS test_workflow_manager test_workflow_orchestrator
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
