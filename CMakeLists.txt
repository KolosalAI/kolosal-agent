cmake_minimum_required(VERSION 3.14)

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

# Feature options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_EXAMPLES "Build example applications" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_LOGGING "Enable logging support" ON)
option(ENABLE_METRICS "Enable metrics collection" ON)
option(ENABLE_HEALTH_MONITORING "Enable health monitoring" ON)
option(USE_SYSTEM_LIBS "Use system-installed libraries when possible" OFF)
option(ENABLE_HTTP_CLIENT "Enable HTTP client functionality (requires CURL)" ON)

# Advanced build options
option(USE_PODOFO "Enable PDF processing with PoDoFo" OFF)
option(ENABLE_CUDA "Enable CUDA support for GPU acceleration" OFF)
option(ENABLE_VULKAN "Enable Vulkan support" OFF)
option(ENABLE_METAL "Enable Metal support (macOS)" OFF)
option(ENABLE_NATIVE_OPTS "Enable native CPU optimizations" OFF)

# Development options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Build configuration options
option(CLEAN_BUILD "Perform clean build" OFF)
option(INSTALL_AFTER_BUILD "Install after build" OFF)
option(PACKAGE_AFTER_BUILD "Create packages after build" OFF)
option(RUN_TESTS_AFTER_BUILD "Run tests after build" OFF)
option(VERBOSE_BUILD "Verbose build output" OFF)

# Print configuration
message(STATUS "========================================")
message(STATUS "Kolosal Agent System v${PROJECT_VERSION}")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Shared Libraries: ${BUILD_SHARED_LIBS}")
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
        
        if(ENABLE_NATIVE_OPTS)
            add_compile_options(-march=native -mtune=native)
        endif()
    endif()
endif()

# Sanitizer support
if(ENABLE_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_TSAN AND NOT MSVC)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

if(ENABLE_UBSAN AND NOT MSVC)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

# Include directories
include_directories(include)
include_directories(kolosal-server/include)

# Find required packages
find_package(Threads REQUIRED)

# External dependencies
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# nlohmann/json
if(USE_SYSTEM_LIBS)
    find_package(nlohmann_json QUIET)
endif()

if(NOT nlohmann_json_FOUND)
    include_directories(${EXTERNAL_DIR}/nlohmann)
endif()

# yaml-cpp (prioritize main external for this project; kolosal-server builds its own in an isolated external build)
if(USE_SYSTEM_LIBS)
    find_package(yaml-cpp QUIET)
endif()

if(NOT yaml-cpp_FOUND)
    # Build yaml-cpp locally for agent only (kolosal-server will build its own copy in an isolated build)
    if(EXISTS "${EXTERNAL_DIR}/yaml-cpp/CMakeLists.txt")
        message(STATUS "Using yaml-cpp from main external directory for kolosal-agent")
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
else()
    set(YAML_CPP_LIBRARIES yaml-cpp)
    set(YAML_CPP_AVAILABLE TRUE)
endif()

# cURL for HTTP client functionality
if(ENABLE_HTTP_CLIENT)
    find_package(CURL QUIET)

    if(NOT CURL_FOUND)
        message(WARNING "CURL not found. Trying alternative approaches...")
        
        if(WIN32)
            # On Windows, try to use the bundled CURL if available
            if(EXISTS "${EXTERNAL_DIR}/curl")
                message(STATUS "Using bundled CURL from external/curl")
                set(CURL_FOUND TRUE)
                set(CURL_INCLUDE_DIRS "${EXTERNAL_DIR}/curl/include")
                set(CURL_LIBRARIES "${EXTERNAL_DIR}/curl/lib/libcurl.lib")
                
                # Verify the bundled files exist
                if(NOT EXISTS "${CURL_INCLUDE_DIRS}/curl/curl.h")
                    message(WARNING "Bundled CURL headers not found at ${CURL_INCLUDE_DIRS}")
                    set(CURL_FOUND FALSE)
                endif()
                
                if(NOT EXISTS "${CURL_LIBRARIES}")
                    # Try alternative library names
                    if(EXISTS "${EXTERNAL_DIR}/curl/lib/curl.lib")
                        set(CURL_LIBRARIES "${EXTERNAL_DIR}/curl/lib/curl.lib")
                    elseif(EXISTS "${EXTERNAL_DIR}/curl/lib/libcurl_a.lib")
                        set(CURL_LIBRARIES "${EXTERNAL_DIR}/curl/lib/libcurl_a.lib")
                    else()
                        message(WARNING "Bundled CURL library not found")
                        set(CURL_FOUND FALSE)
                    endif()
                endif()
            endif()
            
            # If bundled CURL not available, disable HTTP features
            if(NOT CURL_FOUND)
                message(WARNING "CURL not available. HTTP client functionality will be disabled.")
                message(STATUS "To enable HTTP functionality, please install CURL:")
                message(STATUS "  - Using vcpkg: vcpkg install curl")
                message(STATUS "  - Or download pre-built binaries to external/curl/")
                set(CURL_LIBRARIES "")
                set(CURL_INCLUDE_DIRS "")
                add_definitions(-DNO_CURL_SUPPORT)
            endif()
        else()
            # On Unix-like systems
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                pkg_check_modules(CURL libcurl)
            endif()
            
            if(NOT CURL_FOUND)
                message(WARNING "CURL not found. HTTP client functionality will be disabled.")
                message(STATUS "To enable HTTP functionality, please install libcurl-dev:")
                message(STATUS "  - Ubuntu/Debian: sudo apt install libcurl4-openssl-dev")
                message(STATUS "  - CentOS/RHEL: sudo yum install libcurl-devel")
                message(STATUS "  - macOS: brew install curl")
                set(CURL_LIBRARIES "")
                set(CURL_INCLUDE_DIRS "")
                add_definitions(-DNO_CURL_SUPPORT)
            endif()
        endif()
    else()
        message(STATUS "CURL found: ${CURL_LIBRARIES}")
    endif()
else()
    message(STATUS "HTTP client functionality disabled by configuration")
    add_definitions(-DNO_CURL_SUPPORT)
    set(CURL_FOUND FALSE)
    set(CURL_LIBRARIES "")
    set(CURL_INCLUDE_DIRS "")
endif()

# PoDoFo for PDF support (optional)
if(USE_PODOFO)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PODOFO libpodofo)
        if(PODOFO_FOUND)
            add_definitions(-DUSE_PODOFO)
            message(STATUS "PoDoFo support enabled")
        else()
            message(WARNING "PoDoFo requested but not found. PDF support disabled.")
        endif()
    else()
        message(WARNING "PkgConfig not found. Cannot detect PoDoFo. PDF support disabled.")
    endif()
else()
    message(STATUS "PoDoFo support disabled")
endif()

# GPU acceleration support
if(ENABLE_CUDA)
    find_package(CUDA QUIET)
    if(CUDA_FOUND)
        enable_language(CUDA)
        add_definitions(-DUSE_CUDA)
        message(STATUS "CUDA support enabled")
    else()
        message(WARNING "CUDA requested but not found")
    endif()
endif()

if(ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        add_definitions(-DUSE_VULKAN)
        message(STATUS "Vulkan support enabled")
    else()
        message(WARNING "Vulkan requested but not found")
    endif()
endif()

if(ENABLE_METAL AND APPLE)
    add_definitions(-DUSE_METAL)
    message(STATUS "Metal support enabled")
endif()

# Source files organization
set(AGENT_CORE_SOURCES
    src/agent/agent_core.cpp
    src/agent/multi_agent_system.cpp
    src/agent/agent_interfaces.cpp
    src/agent/agent_roles.cpp
    src/agent/agent_data.cpp
    src/agent/agent_config_validator.cpp
    src/agent/agent_factory.cpp
    src/agent/agent_orchestrator.cpp
)

set(ROUTES_SOURCES
    src/routing/message_router.cpp
    src/routing/http_client.cpp
)

set(API_SOURCES
    src/rest_api/agent_management_route.cpp
)

set(SERVICES_SOURCES
    src/agent_services/agent_service.cpp
)

set(INTEGRATION_SOURCES
    src/system_integration/unified_server.cpp
)

set(CORE_SOURCES
    src/yaml_configuration_parser.cpp
    src/function_execution_manager.cpp
    src/task_job_manager.cpp
    src/system_event_manager.cpp
    src/system_tool_registry.cpp
    src/agent_memory_manager.cpp
    src/agent_planning_system.cpp
    src/server_logger_integration.cpp
    src/loading_animation_utils.cpp
    src/server_client_interface.cpp
    src/builtin_function_registry.cpp
    src/document_agent_service.cpp
    src/document_service_manager.cpp
    src/workflow_agent_service.cpp
    src/logger_system.cpp
)

set(SEQUENTIAL_SOURCES
    src/workflow_sequential/sequential_workflow.cpp
    src/workflow_sequential/workflow_engine.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${AGENT_CORE_SOURCES}
    ${ROUTES_SOURCES}
    ${API_SOURCES}
    ${SERVICES_SOURCES}
    ${INTEGRATION_SOURCES}
    ${CORE_SOURCES}
    ${SEQUENTIAL_SOURCES}
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
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/ggml/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/common
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/pugixml/src
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/nlohmann
        ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann
        ${CMAKE_CURRENT_SOURCE_DIR}/external/yaml-cpp/include
)

# Preprocessor definitions
target_compile_definitions(kolosal_agent 
    PRIVATE
        KOLOSAL_AGENT_VERSION="${PROJECT_VERSION}"
        KOLOSAL_AGENT_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        KOLOSAL_AGENT_BUILD  # Define that we're building the library
)

# Handle static vs shared library definitions
if(BUILD_SHARED_LIBS)
    target_compile_definitions(kolosal_agent 
        PUBLIC KOLOSAL_AGENT_SHARED
    )
else()
    target_compile_definitions(kolosal_agent 
        PUBLIC KOLOSAL_AGENT_STATIC
    )
endif()

if(ENABLE_LOGGING)
    target_compile_definitions(kolosal_agent PRIVATE ENABLE_LOGGING)
endif()

if(ENABLE_METRICS)
    target_compile_definitions(kolosal_agent PRIVATE ENABLE_METRICS)
endif()

if(ENABLE_HEALTH_MONITORING)
    target_compile_definitions(kolosal_agent PRIVATE ENABLE_HEALTH_MONITORING)
endif()

# Link libraries
target_link_libraries(kolosal_agent
    PUBLIC
        Threads::Threads
        ${YAML_CPP_LIBRARIES}
        kolosal_server
)

# CURL linking (optional)
if(CURL_FOUND AND CURL_LIBRARIES)
    target_link_libraries(kolosal_agent PRIVATE ${CURL_LIBRARIES})
    target_include_directories(kolosal_agent PRIVATE ${CURL_INCLUDE_DIRS})
    message(STATUS "HTTP client functionality enabled with CURL")
else()
    message(STATUS "HTTP client functionality disabled (CURL not available)")
endif()

# PoDoFo linking
if(PODOFO_FOUND)
    target_link_libraries(kolosal_agent PRIVATE ${PODOFO_LIBRARIES})
    target_include_directories(kolosal_agent PRIVATE ${PODOFO_INCLUDE_DIRS})
endif()

# CUDA linking
if(CUDA_FOUND)
    target_link_libraries(kolosal_agent PRIVATE ${CUDA_LIBRARIES})
    target_include_directories(kolosal_agent PRIVATE ${CUDA_INCLUDE_DIRS})
endif()

# Vulkan linking  
if(Vulkan_FOUND)
    target_link_libraries(kolosal_agent PRIVATE Vulkan::Vulkan)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(kolosal_agent PRIVATE ws2_32 winmm)
elseif(APPLE)
    target_link_libraries(kolosal_agent PRIVATE "-framework Foundation" "-framework CoreFoundation")
    if(ENABLE_METAL)
        target_link_libraries(kolosal_agent PRIVATE "-framework Metal" "-framework MetalKit")
    endif()
else()
    target_link_libraries(kolosal_agent PRIVATE dl rt)
endif()

# Build kolosal-server as an isolated external project (no code changes inside kolosal-server)
include(ExternalProject)
set(KOLOSAL_SERVER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server")
set(KOLOSAL_SERVER_BINARY_DIR "${CMAKE_BINARY_DIR}/_ext/kolosal-server-build")

# Compute OpenSSL paths for kolosal-server's bundled PoDoFo deps (Windows only)
if(WIN32)
    set(_PODOFO_DEPS_DIR "${KOLOSAL_SERVER_SOURCE_DIR}/external/podofo/extern/deps/3rdparty")
    set(_OPENSSL_INCLUDE "${_PODOFO_DEPS_DIR}/openssl/include")
    # Use Debug libraries for Debug config; Release will be handled separately if needed
    set(_OPENSSL_SSL_LIB_DEBUG "${_PODOFO_DEPS_DIR}/openssl/lib/Win64/ssld.lib")
    set(_OPENSSL_CRYPTO_LIB_DEBUG "${_PODOFO_DEPS_DIR}/openssl/lib/Win64/cryptod.lib")
    set(_OPENSSL_SSL_LIB_RELEASE "${_PODOFO_DEPS_DIR}/openssl/lib/Win64/ssl.lib")
    set(_OPENSSL_CRYPTO_LIB_RELEASE "${_PODOFO_DEPS_DIR}/openssl/lib/Win64/crypto.lib")
endif()

ExternalProject_Add(ext_kolosal_server
    SOURCE_DIR "${KOLOSAL_SERVER_SOURCE_DIR}"
    BINARY_DIR "${KOLOSAL_SERVER_BINARY_DIR}"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_GENERATOR_PLATFORM "${CMAKE_VS_PLATFORM_NAME}"
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=ON
        -DUSE_PODOFO=ON
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/cmake/overrides
        
    CMAKE_CACHE_ARGS
        $<$<BOOL:${WIN32}>:-DOPENSSL_ROOT_DIR:PATH=${_PODOFO_DEPS_DIR}/openssl>
        $<$<BOOL:${WIN32}>:-DOPENSSL_INCLUDE_DIR:PATH=${_OPENSSL_INCLUDE}>
        $<$<BOOL:${WIN32}>:-DOPENSSL_SSL_LIBRARY:FILEPATH=${_OPENSSL_SSL_LIB_DEBUG}>
        $<$<BOOL:${WIN32}>:-DOPENSSL_CRYPTO_LIBRARY:FILEPATH=${_OPENSSL_CRYPTO_LIB_DEBUG}>
    $<$<BOOL:${WIN32}>:-DOPENSSL_SSL_LIBRARY_DEBUG:FILEPATH=${_OPENSSL_SSL_LIB_DEBUG}>
    $<$<BOOL:${WIN32}>:-DOPENSSL_SSL_LIBRARY_RELEASE:FILEPATH=${_OPENSSL_SSL_LIB_RELEASE}>
    $<$<BOOL:${WIN32}>:-DOPENSSL_CRYPTO_LIBRARY_DEBUG:FILEPATH=${_OPENSSL_CRYPTO_LIB_DEBUG}>
    $<$<BOOL:${WIN32}>:-DOPENSSL_CRYPTO_LIBRARY_RELEASE:FILEPATH=${_OPENSSL_CRYPTO_LIB_RELEASE}>
    $<$<BOOL:${WIN32}>:-DOPENSSL_LIBRARIES:STRING=${_OPENSSL_SSL_LIB_DEBUG};${_OPENSSL_CRYPTO_LIB_DEBUG}>
    $<$<BOOL:${WIN32}>:-DOPENSSL_USE_STATIC_LIBS:BOOL=TRUE>
    BUILD_ALWAYS OFF
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

# Import the built kolosal_server library for linking
if(WIN32)
    add_library(kolosal_server SHARED IMPORTED)
    # Import library (.lib) and runtime DLL per-config
    set_target_properties(kolosal_server PROPERTIES
        IMPORTED_IMPLIB_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/Debug/kolosal_server.lib"
        IMPORTED_IMPLIB_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.lib"
        IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/Debug/kolosal_server.dll"
        IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/Release/kolosal_server.dll"
    )
else()
    add_library(kolosal_server SHARED IMPORTED)
    if(APPLE)
        set_target_properties(kolosal_server PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libkolosal_server.dylib"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.dylib"
        )
    else()
        set_target_properties(kolosal_server PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libkolosal_server.so"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.so"
        )
    endif()
endif()

# Ensure kolosal-server external build completes before building agent targets
add_dependencies(kolosal_agent ext_kolosal_server)

# Main executable (unified v2.0)
add_executable(kolosal-agent src/main.cpp)
target_link_libraries(kolosal-agent kolosal_agent)
set_target_properties(kolosal-agent PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "kolosal-agent-unified"
)

# Launcher executable
add_executable(kolosal-launcher src/kolosal_launcher.c)
if(WIN32)
    target_link_libraries(kolosal-launcher kernel32 user32)
endif()

# On Windows, copy kolosal_server.dll next to the agent executable for runtime
if(WIN32)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:kolosal_server>"
            "$<TARGET_FILE_DIR:kolosal-agent>/"
        COMMENT "Copying kolosal_server DLL to agent output directory"
    )
endif()

# Installation configuration
include(GNUInstallDirs)

# Install binaries
install(TARGETS kolosal-agent kolosal-launcher kolosal_agent
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
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

# Install documentation
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    install(FILES README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )
endif()

# Tests configuration
if(BUILD_TESTS)
    enable_testing()
    
    # Find test framework
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_subdirectory(tests)
        message(STATUS "Tests enabled with Google Test")
    else()
        message(WARNING "Tests requested but Google Test not found")
    endif()
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
    message(STATUS "Examples enabled")
endif()

# Documentation
if(BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(Doxygen_FOUND)
        configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(docs
            ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
        )
        message(STATUS "Documentation enabled")
    else()
        message(WARNING "Documentation requested but Doxygen not found")
    endif()
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
    set(CPACK_NSIS_HELP_LINK "https://github.com/Evintkoo/kolosal-agent")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/Evintkoo/kolosal-agent")
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

# Custom targets to replace build script functionality

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

# Post-build actions
if(INSTALL_AFTER_BUILD)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}
        COMMENT "Installing after build"
    )
endif()

if(PACKAGE_AFTER_BUILD)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package
        COMMENT "Creating packages after build"
    )
endif()

# Run tests after build
if(RUN_TESTS_AFTER_BUILD AND BUILD_TESTS)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests after build"
    )
endif()

# Git submodule initialization target (optional)
option(INIT_SUBMODULES "Initialize git submodules on configure/build" ON)
find_package(Git QUIET)
if(Git_FOUND AND INIT_SUBMODULES)
    add_custom_target(init-submodules
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Initializing git submodules"
    )

    # Make main targets depend on submodule initialization
    add_dependencies(kolosal_agent init-submodules)
endif()

# Verbose build configuration
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Summary
message(STATUS "========================================")
message(STATUS "Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared Libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Documentation: ${BUILD_DOCS}")
message(STATUS "  Features:")
message(STATUS "    - Logging: ${ENABLE_LOGGING}")
message(STATUS "    - Metrics: ${ENABLE_METRICS}")
message(STATUS "    - Health Monitoring: ${ENABLE_HEALTH_MONITORING}")
message(STATUS "    - HTTP Client: ${CURL_FOUND}")
message(STATUS "    - PoDoFo PDF: ${USE_PODOFO}")
message(STATUS "    - CUDA: ${ENABLE_CUDA}")
message(STATUS "    - Vulkan: ${ENABLE_VULKAN}")
message(STATUS "    - Metal: ${ENABLE_METAL}")
message(STATUS "========================================")
message(STATUS "")
message(STATUS "🔧 Build Instructions:")
message(STATUS "  Configure with Debug build:")
message(STATUS "    cmake .. -DCMAKE_BUILD_TYPE=Debug")
message(STATUS "  ")
message(STATUS "  Build with Debug configuration:")
message(STATUS "    cmake --build . --config Debug")
message(STATUS "  ")
message(STATUS "  Additional options:")
message(STATUS "    cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON")
message(STATUS "    cmake --build . --config Debug --parallel")
message(STATUS "  ")
if(NOT CURL_FOUND)
message(STATUS "  💡 To enable HTTP client functionality:")
message(STATUS "    Install CURL and reconfigure:")
if(WIN32)
message(STATUS "      vcpkg install curl")
message(STATUS "      cmake .. -DCURL_ROOT=path/to/curl")
else()
message(STATUS "      sudo apt install libcurl4-openssl-dev  # Ubuntu/Debian")
message(STATUS "      sudo yum install libcurl-devel         # CentOS/RHEL")  
message(STATUS "      brew install curl                      # macOS")
endif()
message(STATUS "  ")
endif()
message(STATUS "  Run tests:")
message(STATUS "    ctest --output-on-failure")
message(STATUS "  ")
message(STATUS "  Install:")
message(STATUS "    cmake --install . --config Debug")
message(STATUS "  ")
message(STATUS "  Create packages:")
message(STATUS "    cpack")
message(STATUS "  ")
message(STATUS "  Clean all:")
message(STATUS "    cmake --build . --target clean-all")
message(STATUS "  ")
message(STATUS "  System info:")
message(STATUS "    cmake --build . --target info")
message(STATUS "========================================")
message(STATUS "")
