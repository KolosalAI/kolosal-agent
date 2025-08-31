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
include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/external/googletest/googlemock/include)

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

# Define test sources
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