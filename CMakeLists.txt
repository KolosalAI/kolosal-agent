cmake_minimum_required(VERSION 3.14...4.0)

# Force set policy version to avoid CMake 4.0 compatibility issues
set(CMAKE_POLICY_VERSION_MINIMUM 3.14)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

# Set policy defaults for CMake 4.0 compatibility
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# Project configuration
project(KolosalAgent 
    VERSION 2.0.0
    LANGUAGES CXX C
    DESCRIPTION "Advanced Multi-Agent AI System with Unified LLM Server Integration"
)

# C++ Standards and compiler requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
    add_definitions(-DPLATFORM_MACOS)
else()
    set(PLATFORM_NAME "Linux")
    add_definitions(-DPLATFORM_LINUX)
endif()

# Build mode options (Simplified to 3 modes)
option(BUILD_TESTS "Build with all tests (unit, integration, performance)" OFF)
option(BUILD_DEEPRESEARCH_DEMO "Build deep research demo" OFF)
option(ENABLE_LOGGING "Enable logging support" ON)

# Configure build mode (Simplified - 3 modes)
if(BUILD_TESTS)
    message(STATUS "🧪 Test Mode Enabled - Building with all tests")
    set(BUILD_UNIT_TESTS ON)
    set(BUILD_INTEGRATION_TESTS ON)
    set(BUILD_PERFORMANCE_TESTS ON)
    set(ENABLE_LOGGING ON)
elseif(BUILD_DEEPRESEARCH_DEMO)
    message(STATUS "🔬 Deep Research Demo Mode Enabled")
    set(BUILD_UNIT_TESTS OFF)
    set(BUILD_INTEGRATION_TESTS OFF)
    set(BUILD_PERFORMANCE_TESTS OFF)
    set(ENABLE_LOGGING ON)
else()
    message(STATUS "📦 Standard Build Mode - No tests, no demo")
    set(BUILD_UNIT_TESTS OFF)
    set(BUILD_INTEGRATION_TESTS OFF)
    set(BUILD_PERFORMANCE_TESTS OFF)
endif()

# Print configuration
message(STATUS "========================================")
message(STATUS "Kolosal Agent System v${PROJECT_VERSION}")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(BUILD_TESTS)
message(STATUS "🧪 TEST MODE: ENABLED (All tests included)")
elseif(BUILD_DEEPRESEARCH_DEMO)
message(STATUS "🔬 DEEP RESEARCH DEMO MODE: ENABLED")
else()
message(STATUS "📦 STANDARD BUILD: Default mode")
endif()
message(STATUS "========================================")

# Compiler-specific settings
if(MSVC)
    # Visual Studio specific settings
    add_compile_options(/W4 /WX- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
    
    # Enable C++17 features explicitly
    add_compile_options(/std:c++17)
    
    # Enable parallel compilation
    add_compile_options(/MP)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/MDd /Od /Zi)
    else()
        add_compile_options(/MD /O2)
    endif()
else()
    # GCC/Clang settings
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Include directories
include_directories(include)
include_directories(kolosal-server/include)
include_directories(kolosal-server/inference/include)

# Find required packages
find_package(Threads REQUIRED)

# External dependencies
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# nlohmann/json
include_directories(${EXTERNAL_DIR}/nlohmann)

# yaml-cpp
if(EXISTS "${EXTERNAL_DIR}/yaml-cpp/CMakeLists.txt")
    message(STATUS "Using yaml-cpp from external directory")
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")
    set(YAML_CPP_INSTALL OFF CACHE BOOL "disable yaml install")
    add_subdirectory(${EXTERNAL_DIR}/yaml-cpp EXCLUDE_FROM_ALL)
    set(YAML_CPP_LIBRARIES yaml-cpp)
    set(YAML_CPP_AVAILABLE TRUE)
else()
    message(WARNING "yaml-cpp not found in external directory")
    set(YAML_CPP_AVAILABLE FALSE)
endif()

# cURL for HTTP client functionality (optional)
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    message(WARNING "CURL not found. HTTP client functionality will be disabled.")
    add_definitions(-DNO_CURL_SUPPORT)
    set(CURL_LIBRARIES "")
    set(CURL_INCLUDE_DIRS "")
else()
    message(STATUS "CURL found: ${CURL_LIBRARIES}")
endif()

# Source files organization
set(AGENT_CORE_SOURCES
    src/agent/core/agent_core.cpp
    src/agent/core/multi_agent_system.cpp
    src/agent/core/agent_interfaces.cpp
    src/agent/core/agent_roles.cpp
    src/agent/core/agent_data.cpp
    src/agent/core/agent_config_validator.cpp
    src/agent/core/agent_factory.cpp
    src/agent/core/agent_orchestrator.cpp
)

set(AGENT_MEMORY_SOURCES
    src/agent/memory/agent_memory_manager.cpp
)

set(AGENT_PLANNING_SOURCES
    src/agent/planning/agent_planning_system.cpp
)

set(AGENT_SERVICES_SOURCES
    src/agent/services/agent_service.cpp
    src/agent/services/document_agent_service.cpp
    src/agent/services/workflow_agent_service.cpp
)

set(API_SOURCES
    src/api/agent_management_route.cpp
    src/api/simple_http_server.cpp
    src/api/message_router.cpp
    src/api/http_client.cpp
)

set(CONFIG_SOURCES
    src/config/yaml_configuration_parser.cpp
)

set(CORE_SOURCES
    # Main executables are handled separately
)

set(EXECUTION_SOURCES
    src/execution/function_execution_manager.cpp
    src/execution/task_job_manager.cpp
)

set(LOGGER_SOURCES
    src/logger/logger_system.cpp
    src/logger/server_logger_integration.cpp
)

set(SERVER_SOURCES
    src/server/server_client_interface.cpp
    src/server/unified_server.cpp
)

set(TOOLS_SOURCES
    src/tools/system_event_manager.cpp
    src/tools/system_tool_registry.cpp
    src/tools/builtin_function_registry.cpp
    src/tools/kolosal_server_functions.cpp
    src/tools/enhanced_function_registry.cpp
    src/tools/research_functions.cpp
)

set(UTILS_SOURCES
    src/utils/document_service_manager.cpp
    src/utils/loading_animation_utils.cpp
)

set(WORKFLOW_SOURCES
    src/workflow/sequential_workflow.cpp
    src/workflow/workflow_engine.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${AGENT_CORE_SOURCES}
    ${AGENT_MEMORY_SOURCES}
    ${AGENT_PLANNING_SOURCES}
    ${AGENT_SERVICES_SOURCES}
    ${API_SOURCES}
    ${CONFIG_SOURCES}
    ${EXECUTION_SOURCES}
    ${LOGGER_SOURCES}
    ${SERVER_SOURCES}
    ${TOOLS_SOURCES}
    ${UTILS_SOURCES}
    ${WORKFLOW_SOURCES}
)

# Create the main library
add_library(kolosal_agent ${ALL_SOURCES})

# Library properties
set_target_properties(kolosal_agent PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Include directories for the kolosal_agent target
target_include_directories(kolosal_agent 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann
        ${CMAKE_CURRENT_SOURCE_DIR}/external/yaml-cpp/include
)

# Preprocessor definitions
target_compile_definitions(kolosal_agent 
    PRIVATE
        KOLOSAL_AGENT_VERSION="${PROJECT_VERSION}"
        KOLOSAL_AGENT_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        KOLOSAL_AGENT_BUILD
        KOLOSAL_SERVER_BUILD
)

# Test mode specific definitions
if(BUILD_TESTS)
    target_compile_definitions(kolosal_agent 
        PRIVATE
            KOLOSAL_AGENT_TEST_MODE
            KOLOSAL_AGENT_DEBUG_VERBOSE
    )
endif()

# Deep research demo specific definitions
if(BUILD_DEEPRESEARCH_DEMO)
    target_compile_definitions(kolosal_agent 
        PRIVATE
            KOLOSAL_AGENT_DEEPRESEARCH_MODE
    )
endif()

if(ENABLE_LOGGING)
    target_compile_definitions(kolosal_agent PRIVATE ENABLE_LOGGING)
endif()

# Link libraries
target_link_libraries(kolosal_agent
    PUBLIC
        Threads::Threads
        ${YAML_CPP_LIBRARIES}
        kolosal_server
)

# Windows system libraries
if(WIN32)
    target_link_libraries(kolosal_agent PRIVATE
        ws2_32
        winmm
        crypt32
        advapi32
        user32
        gdi32
        ole32
        oleaut32
        uuid
        shell32
        comdlg32
        winspool
    )
endif()

# CURL linking (optional)
if(CURL_FOUND AND CURL_LIBRARIES)
    target_link_libraries(kolosal_agent PRIVATE ${CURL_LIBRARIES})
    target_include_directories(kolosal_agent PRIVATE ${CURL_INCLUDE_DIRS})
    message(STATUS "HTTP client functionality enabled with CURL")
else()
    message(STATUS "HTTP client functionality disabled (CURL not available)")
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(kolosal_agent PRIVATE ws2_32 winmm)
elseif(APPLE)
    target_link_libraries(kolosal_agent PRIVATE "-framework Foundation" "-framework CoreFoundation")
else()
    target_link_libraries(kolosal_agent PRIVATE dl rt)
endif()

# Build kolosal-server as an external project
include(ExternalProject)
set(KOLOSAL_SERVER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server")
set(KOLOSAL_SERVER_BINARY_DIR "${CMAKE_BINARY_DIR}/kolosal-server")

ExternalProject_Add(ext_kolosal_server
    SOURCE_DIR "${KOLOSAL_SERVER_SOURCE_DIR}"
    BINARY_DIR "${KOLOSAL_SERVER_BINARY_DIR}"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_GENERATOR_PLATFORM "${CMAKE_VS_PLATFORM_NAME}"
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DUSE_PODOFO=OFF
    BUILD_ALWAYS OFF
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

# Add a custom target to copy the kolosal-server executable after build
if(WIN32)
    set(KOLOSAL_SERVER_EXE_PATH "${KOLOSAL_SERVER_BINARY_DIR}/$<CONFIG>/kolosal-server.exe")
    set(KOLOSAL_SERVER_COPY_PATH "${CMAKE_BINARY_DIR}/$<CONFIG>/kolosal-server.exe")
else()
    set(KOLOSAL_SERVER_EXE_PATH "${KOLOSAL_SERVER_BINARY_DIR}/kolosal-server")
    set(KOLOSAL_SERVER_COPY_PATH "${CMAKE_BINARY_DIR}/kolosal-server")
endif()

add_custom_target(kolosal-server-exe
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${KOLOSAL_SERVER_EXE_PATH}"
        "${KOLOSAL_SERVER_COPY_PATH}"
    DEPENDS ext_kolosal_server
    COMMENT "Copying kolosal-server executable to output directory"
)

# Import the built kolosal_server library for linking  
if(WIN32)
    add_library(kolosal_server SHARED IMPORTED)
    set_target_properties(kolosal_server PROPERTIES
        IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/Debug/kolosal_server.dll"
        IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.dll"
        IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.dll"
        IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.dll"
        IMPORTED_IMPLIB_DEBUG           "${KOLOSAL_SERVER_BINARY_DIR}/Debug/kolosal_server.lib"
        IMPORTED_IMPLIB_RELEASE         "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.lib"
        IMPORTED_IMPLIB_RELWITHDEBINFO  "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.lib"
        IMPORTED_IMPLIB_MINSIZEREL      "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.lib"
    )
else()
    add_library(kolosal_server STATIC IMPORTED)
    set_target_properties(kolosal_server PROPERTIES
        IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libkolosal_server.a"
        IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
        IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
        IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
    )
endif()

# Ensure kolosal-server external build completes before building agent targets
add_dependencies(kolosal_agent ext_kolosal_server)
set(KOLOSAL_SERVER_AVAILABLE TRUE)

# Main executable
add_executable(kolosal-agent src/core/main.cpp)
target_link_libraries(kolosal-agent kolosal_agent)
set_target_properties(kolosal-agent PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "kolosal-agent"
)

# Deep Research Demo executable (optional)
if(BUILD_DEEPRESEARCH_DEMO)
    add_executable(deep-research-demo examples/deep_research_agent_demo.cpp)
    target_link_libraries(deep-research-demo kolosal_agent)
    set_target_properties(deep-research-demo PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        OUTPUT_NAME "deep-research-demo"
    )
    
    # Make sure kolosal-server executable is built
    add_dependencies(deep-research-demo kolosal-server-exe)
    
    # Copy required DLLs for deep-research-demo (Windows)
    if(WIN32)
        add_custom_command(TARGET deep-research-demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/kolosal-server/Debug/kolosal_server.dll
                $<TARGET_FILE_DIR:deep-research-demo>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/kolosal-server/Debug/libcurl.dll
                $<TARGET_FILE_DIR:deep-research-demo>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/kolosal-server/Debug/llama-cpu.dll
                $<TARGET_FILE_DIR:deep-research-demo>
            COMMENT "Copying required DLLs for deep-research-demo.exe"
            VERBATIM
        )
    endif()
    
    # Ensure both main executables are built by default (ALL target)
    add_custom_target(ALL_BUILD_TARGETS ALL
        DEPENDS kolosal-agent kolosal-server-exe deep-research-demo
        COMMENT "Building all main targets: kolosal-agent, kolosal-server, and deep-research-demo"
    )
else()
    # Standard build targets
    add_custom_target(ALL_BUILD_TARGETS ALL
        DEPENDS kolosal-agent kolosal-server-exe
        COMMENT "Building main targets: kolosal-agent and kolosal-server"
    )
endif()

# Make sure kolosal-server executable is built by default
add_dependencies(kolosal-agent kolosal-server-exe)

# Copy required DLLs for kolosal-agent (Windows)
if(WIN32)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/kolosal_server.dll
            $<TARGET_FILE_DIR:kolosal-agent>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/libcurl.dll
            $<TARGET_FILE_DIR:kolosal-agent>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/llama-cpu.dll
            $<TARGET_FILE_DIR:kolosal-agent>
        COMMENT "Copying required DLLs for kolosal-agent.exe"
        VERBATIM
    )
endif()

# Installation configuration
include(GNUInstallDirs)

# Install binaries
install(TARGETS kolosal-agent kolosal_agent
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install kolosal-server executable
install(FILES "${KOLOSAL_SERVER_COPY_PATH}"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE 
                WORLD_READ WORLD_EXECUTE
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kolosal-agent
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Install configuration files
install(FILES config.example.yaml
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/kolosal-agent
    RENAME config.yaml
)

# Tests configuration (only when BUILD_TESTS=ON)
if(BUILD_TESTS)
    enable_testing()
    
    # Find GoogleTest
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        # Use local GoogleTest from external directory
        if(EXISTS "${CMAKE_SOURCE_DIR}/external/googletest/CMakeLists.txt")
            message(STATUS "Using local GoogleTest from external/googletest")
            
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
            set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
            
            add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest EXCLUDE_FROM_ALL)
            
            if(NOT TARGET GTest::gtest)
                add_library(GTest::gtest ALIAS gtest)
            endif()
            if(NOT TARGET GTest::gtest_main)
                add_library(GTest::gtest_main ALIAS gtest_main)
            endif()
            
            set(GTest_FOUND TRUE)
            message(STATUS "GoogleTest configured from external/googletest")
        else()
            message(WARNING "GoogleTest not found in external/googletest directory")
        endif()
    endif()
    
    if(GTest_FOUND)
        add_subdirectory(tests)
        message(STATUS "🧪 All tests enabled with Google Test")
    else()
        message(WARNING "Tests requested but Google Test not found")
    endif()
else()
    message(STATUS "📦 Tests disabled - standard build mode")
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Kolosal AI")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Kolosal Agent System")
    set(CPACK_NSIS_PACKAGE_NAME "Kolosal Agent System")
    set(CPACK_NSIS_HELP_LINK "https://github.com/kolosalai/kolosal-agent")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/kolosalai/kolosal-agent")
    set(CPACK_NSIS_CONTACT "support@kolosal.ai")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kolosal AI <support@kolosal.ai>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libcurl4, libyaml-cpp0.6")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++, libcurl, yaml-cpp")
endif()

include(CPack)

# Custom targets

# Clean all target
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/CMakeCache.txt
    COMMENT "Cleaning all build artifacts and CMake cache"
)

# System info target
add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo "========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Kolosal Agent System v${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Platform: ${PLATFORM_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build Type: ${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "C++ Standard: ${CMAKE_CXX_STANDARD}"
    COMMAND ${CMAKE_COMMAND} -E echo "CMake Version: ${CMAKE_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Generator: ${CMAKE_GENERATOR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Source Dir: ${CMAKE_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Binary Dir: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "========================================="
    COMMENT "Displaying system and build information"
)

# Build only kolosal-server target
add_custom_target(build-server
    DEPENDS kolosal-server-exe
    COMMENT "Building only the kolosal-server executable"
)

# Build only kolosal-agent target
add_custom_target(build-agent
    DEPENDS kolosal-agent
    COMMENT "Building only the kolosal-agent executable"
)

# Build both main targets
add_custom_target(build-all
    DEPENDS kolosal-agent kolosal-server-exe
    COMMENT "Building both kolosal-agent and kolosal-server executables"
)

# Summary
message(STATUS "========================================")
message(STATUS "Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
if(BUILD_TESTS)
message(STATUS "  🧪 Build Mode: WITH ALL TESTS")
message(STATUS "    - Unit Tests: YES")
message(STATUS "    - Integration Tests: YES")
message(STATUS "    - Performance Tests: YES")
elseif(BUILD_DEEPRESEARCH_DEMO)
message(STATUS "  🔬 Build Mode: DEEP RESEARCH DEMO")
message(STATUS "    - Deep Research Demo: YES")
else()
message(STATUS "  📦 Build Mode: STANDARD (Default)")
endif()
message(STATUS "  Features:")
message(STATUS "    - Logging: ${ENABLE_LOGGING}")
message(STATUS "    - HTTP Client: ${CURL_FOUND}")
message(STATUS "========================================")
message(STATUS "")
message(STATUS "🔧 Build Instructions:")
message(STATUS "")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  📦 STANDARD BUILD (Default)")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake .. -DCMAKE_BUILD_TYPE=Debug")
message(STATUS "    cmake --build . --config Debug")
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🧪 BUILD WITH ALL TESTS")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON")
message(STATUS "    cmake --build . --config Debug")
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🔬 BUILD WITH DEEP RESEARCH DEMO")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_DEEPRESEARCH_DEMO=ON")
message(STATUS "    cmake --build . --config Debug")
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  📁 OUTPUT EXECUTABLES")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    Location: build/Debug/ or build/")
message(STATUS "    - kolosal-agent(.exe) - Main agent system")
message(STATUS "    - kolosal-server(.exe) - Standalone LLM server")
if(BUILD_DEEPRESEARCH_DEMO)
message(STATUS "    - deep-research-demo(.exe) - Deep research demo")
endif()
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🎯 BUILD TARGETS")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake --build . --config Debug                     # Build everything")
message(STATUS "    cmake --build . --target build-agent --config Debug    # Build only kolosal-agent")
message(STATUS "    cmake --build . --target build-server --config Debug   # Build only kolosal-server")
message(STATUS "    cmake --build . --target build-all --config Debug      # Build both main executables")
if(BUILD_TESTS)
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🧪 TEST TARGETS (Available)")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    ctest --output-on-failure                              # Run tests manually")
endif()
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🛠️  UTILITY TARGETS")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake --build . --target clean-all      # Clean everything")
message(STATUS "    cmake --build . --target info           # Show system info")
message(STATUS "    cmake --install . --config Debug        # Install binaries")
message(STATUS "    cpack                                    # Create packages")
message(STATUS "  ")
message(STATUS "========================================")
