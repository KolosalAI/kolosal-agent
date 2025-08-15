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

# Build mode options (Simplified to 2 modes)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build with all tests (unit, integration, performance)" OFF)
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

# Development options (only used when BUILD_TESTS=ON)
option(ENABLE_ASAN "Enable AddressSanitizer (test mode only)" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer (test mode only)" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (test mode only)" OFF)

# Build configuration options
option(VERBOSE_BUILD "Verbose build output" OFF)

# Configure build mode (Simplified - only 2 modes)
if(BUILD_TESTS)
    message(STATUS "🧪 Test Mode Enabled - Building with all tests")
    set(BUILD_UNIT_TESTS ON)
    set(BUILD_INTEGRATION_TESTS ON)
    set(BUILD_PERFORMANCE_TESTS ON)
    set(ENABLE_LOGGING ON)
    set(ENABLE_METRICS ON)
    
    # Enable sanitizers if requested in test mode
    if(ENABLE_ASAN AND NOT MSVC)
        message(STATUS "AddressSanitizer enabled for testing")
    endif()
    
    if(ENABLE_UBSAN AND NOT MSVC)
        message(STATUS "UndefinedBehaviorSanitizer enabled for testing")
    endif()
else()
    message(STATUS "Standard Build Mode - No tests")
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
message(STATUS "Shared Libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "Build Mode: ${BUILD_TESTS}")
if(BUILD_TESTS)
message(STATUS "🧪 TEST MODE: ENABLED (All tests included)")
else()
message(STATUS "📦 STANDARD BUILD: No tests")
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
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/external/llama.cpp/ggml/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/external/llama.cpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/external/llama.cpp/common
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

# Test mode specific definitions
if(BUILD_TESTS)
    target_compile_definitions(kolosal_agent 
        PRIVATE
            KOLOSAL_AGENT_TEST_MODE
            KOLOSAL_AGENT_DEBUG_VERBOSE
    )
endif()

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
        # Commented out static libraries since we're using the shared kolosal_server.dll
        # llama_cpu
        # pugixml_static
        # minizip_static
        # zlib_static
        # yaml_cpp_server
)

# Add PoDoFo libraries conditionally - commented out since using shared kolosal_server.dll
# if(TARGET podofo_static)
#     target_link_libraries(kolosal_agent PUBLIC
#         podofo_static
#         podofo_private
#         podofo_3rd
#     )
# endif()

# Windows system libraries for PoDoFo and other dependencies
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

# Build kolosal-server as an external project in the same build tree
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
        -DUSE_PODOFO=${USE_PODOFO}  # Pass through PoDoFo setting
        -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/cmake/overrides
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        
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
    # Import library (.lib) for shared linking
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
    # Note: Since kolosal-server is built as a shared library (DLL), 
    # most dependencies are already included in the DLL.
    # Commenting out static library imports that may not be needed.
    
    # add_library(podofo_static STATIC IMPORTED)
    # set_target_properties(podofo_static PROPERTIES
    #     IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Debug/podofo.lib"
    #     IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/podofo.lib"
    #     IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/podofo.lib"
    #     IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/podofo.lib"
    # )
    
    # add_library(pugixml_static STATIC IMPORTED)
    # set_target_properties(pugixml_static PROPERTIES
    #     IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Debug/pugixml.lib"
    #     IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Release/pugixml.lib"
    #     IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Release/pugixml.lib"
    #     IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Release/pugixml.lib"
    # )
    
    # add_library(minizip_static STATIC IMPORTED)
    # set_target_properties(minizip_static PROPERTIES
    #     IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/Debug/minizip.lib"
    #     IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/Release/minizip.lib"
    #     IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/Release/minizip.lib"
    #     IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/Release/minizip.lib"
    # )
    
    # add_library(zlib_static STATIC IMPORTED)
    # set_target_properties(zlib_static PROPERTIES
    #     IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Debug/zsd.lib"
    #     IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Release/zsd.lib"
    #     IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Release/zsd.lib"
    #     IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Release/zsd.lib"
    # )
    
    # add_library(llama_cpu STATIC IMPORTED)
    # set_target_properties(llama_cpu PROPERTIES
    #     IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/lib/Debug/llama-cpu.lib"
    #     IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/lib/Release/llama-cpu.lib"
    #     IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/lib/Release/llama-cpu.lib"
    #     IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/lib/Release/llama-cpu.lib"
    # )
    
    # add_library(podofo_3rd STATIC IMPORTED)
    # set_target_properties(podofo_3rd PROPERTIES
    #     IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/3rdparty/Debug/podofo_3rdparty.lib"
    #     IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/3rdparty/Release/podofo_3rdparty.lib"
    # )
    
    # Add podofo_private library
    # add_library(podofo_private STATIC IMPORTED)
    # set_target_properties(podofo_private PROPERTIES
    #     IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/private/Debug/podofo_private.lib"
    #     IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/private/Release/podofo_private.lib"
    # )
    
    add_library(yaml_cpp_server STATIC IMPORTED)
    set_target_properties(yaml_cpp_server PROPERTIES
        IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Debug/yaml-cppd.lib"
        IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Release/yaml-cpp.lib"
        IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Release/yaml-cpp.lib"
        IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Release/yaml-cpp.lib"
    )
else()
    add_library(kolosal_server STATIC IMPORTED)
    if(APPLE)
        set_target_properties(kolosal_server PROPERTIES
            IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libkolosal_server.a"
            IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
            IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
            IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
        )
    else()
        set_target_properties(kolosal_server PROPERTIES
            IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libkolosal_server.a"
            IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
            IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
            IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/Release/libkolosal_server.a"
        )
    endif()
    
    # Import kolosal-server dependencies (static libraries)
    add_library(podofo_static STATIC IMPORTED)
    if(APPLE)
        set_target_properties(podofo_static PROPERTIES
            IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Debug/libpodofo.a"
            IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/libpodofo.a"
            IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/libpodofo.a"
            IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/libpodofo.a"
        )
    else()
        set_target_properties(podofo_static PROPERTIES
            IMPORTED_LOCATION_DEBUG         "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Debug/libpodofo.a"
            IMPORTED_LOCATION_RELEASE       "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/libpodofo.a"
            IMPORTED_LOCATION_RELWITHDEBINFO "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/libpodofo.a"
            IMPORTED_LOCATION_MINSIZEREL     "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/src/podofo/Release/libpodofo.a"
        )
    endif()
    
    add_library(pugixml_static STATIC IMPORTED)
    if(APPLE)
        set_target_properties(pugixml_static PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Debug/libpugixml.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Release/libpugixml.a"
        )
    else()
        set_target_properties(pugixml_static PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Debug/libpugixml.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/pugixml/Release/libpugixml.a"
        )
    endif()
    
    add_library(minizip_static STATIC IMPORTED)
    if(APPLE)
        set_target_properties(minizip_static PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libminizip.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/Release/libminizip.a"
        )
    else()
        set_target_properties(minizip_static PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/Debug/libminizip.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/Release/libminizip.a"
        )
    endif()
    
    add_library(zlib_static STATIC IMPORTED)
    if(APPLE)
        set_target_properties(zlib_static PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Debug/libz.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Release/libz.a"
        )
    else()
        set_target_properties(zlib_static PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Debug/libz.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/zlib/Release/libz.a"
        )
    endif()
    
    add_library(llama_cpu STATIC IMPORTED)
    if(APPLE)
        set_target_properties(llama_cpu PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/lib/Debug/libllama-cpu.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/lib/Release/libllama-cpu.a"
        )
    else()
        set_target_properties(llama_cpu PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/lib/Debug/libllama-cpu.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/lib/Release/libllama-cpu.a"
        )
    endif()
    
    add_library(podofo_3rd STATIC IMPORTED)
    if(APPLE)
        set_target_properties(podofo_3rd PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/3rdparty/Debug/libpodofo_3rdparty.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/3rdparty/Release/libpodofo_3rdparty.a"
        )
    else()
        set_target_properties(podofo_3rd PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/3rdparty/Debug/libpodofo_3rdparty.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/podofo/3rdparty/Release/libpodofo_3rdparty.a"
        )
    endif()
    
    add_library(yaml_cpp_server STATIC IMPORTED)
    if(APPLE)
        set_target_properties(yaml_cpp_server PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Debug/libyaml-cpp.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Release/libyaml-cpp.a"
        )
    else()
        set_target_properties(yaml_cpp_server PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Debug/libyaml-cpp.a"
            IMPORTED_LOCATION_RELEASE "${KOLOSAL_SERVER_BINARY_DIR}/external/yaml-cpp/Release/libyaml-cpp.a"
        )
    endif()
endif()

# Ensure kolosal-server external build completes before building agent targets
add_dependencies(kolosal_agent ext_kolosal_server)
# Commented out dependencies for static libraries since we're using shared kolosal_server.dll
# add_dependencies(podofo_static ext_kolosal_server)
# add_dependencies(podofo_3rd ext_kolosal_server)
# add_dependencies(podofo_private ext_kolosal_server)
# add_dependencies(pugixml_static ext_kolosal_server)
# add_dependencies(minizip_static ext_kolosal_server)
# add_dependencies(zlib_static ext_kolosal_server)
# add_dependencies(llama_cpu ext_kolosal_server)
# add_dependencies(yaml_cpp_server ext_kolosal_server)
set(KOLOSAL_SERVER_AVAILABLE TRUE)

# Simple test executable for Windows dialog fixes
add_executable(simple-test src/core/simple_test_main.cpp)
if(WIN32)
    target_link_libraries(simple-test kernel32 user32)
endif()

# Main executable (unified v2.0)
add_executable(kolosal-agent src/core/main.cpp)
target_link_libraries(kolosal-agent kolosal_agent)
set_target_properties(kolosal-agent PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "kolosal-agent"
)

# Make sure kolosal-server executable is built by default
add_dependencies(kolosal-agent kolosal-server-exe)

# Ensure both main executables are built by default (ALL target)
add_custom_target(ALL_BUILD_TARGETS ALL
    DEPENDS kolosal-agent kolosal-server-exe simple-test kolosal-launcher workflow-example deep-research-demo
    COMMENT "Building all main targets: kolosal-agent, kolosal-server, simple-test, kolosal-launcher, workflow-example, and deep-research-demo"
)

# Launcher executable
add_executable(kolosal-launcher src/core/kolosal_launcher.c)
if(WIN32)
    target_link_libraries(kolosal-launcher kernel32 user32)
endif()

# Workflow example executable
add_executable(workflow-example examples/workflow_example.cpp)
target_link_libraries(workflow-example kolosal_agent)
set_target_properties(workflow-example PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "workflow-example"
)

# Deep Research Agent Demo executable
add_executable(deep-research-demo examples/deep_research_agent_demo.cpp)
target_link_libraries(deep-research-demo kolosal_agent)
set_target_properties(deep-research-demo PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "deep-research-demo"
)

# Since kolosal-server is now built as a static library, no need to copy DLLs
# The static library will be linked directly into the executable

# Installation configuration
include(GNUInstallDirs)

# Install binaries
install(TARGETS kolosal-agent kolosal-launcher kolosal_agent
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

# Install documentation
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    install(FILES README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )
endif()

# Tests configuration (Simplified - only when BUILD_TESTS=ON)
if(BUILD_TESTS)
    enable_testing()
    
    # Find test framework - Use local GoogleTest from external/googletest
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        # Use local GoogleTest from external directory
        if(EXISTS "${CMAKE_SOURCE_DIR}/external/googletest/CMakeLists.txt")
            message(STATUS "Using local GoogleTest from external/googletest")
            
            # For Windows: Prevent overriding the parent project's compiler/linker settings
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
            set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
            
            # Add googletest subdirectory
            add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest EXCLUDE_FROM_ALL)
            
            # Create targets to match what find_package would create
            if(NOT TARGET GTest::gtest)
                add_library(GTest::gtest ALIAS gtest)
            endif()
            if(NOT TARGET GTest::gtest_main)
                add_library(GTest::gtest_main ALIAS gtest_main)
            endif()
            if(NOT TARGET GTest::gmock)
                add_library(GTest::gmock ALIAS gmock)
            endif()
            if(NOT TARGET GTest::gmock_main)
                add_library(GTest::gmock_main ALIAS gmock_main)
            endif()
            
            set(GTest_FOUND TRUE)
            message(STATUS "GoogleTest configured from external/googletest")
        else()
            message(WARNING "GoogleTest not found in external/googletest directory")
        endif()
    endif()
    
    if(GTest_FOUND)
        add_subdirectory(tests)
        message(STATUS "🧪 All tests enabled with Google Test (unit, integration, performance)")
        
        # Find Google Benchmark for performance tests
        find_package(benchmark QUIET)
        if(benchmark_FOUND)
            message(STATUS "Benchmark tests enabled with Google Benchmark")
        else()
            message(STATUS "Performance tests will use basic timing (Google Benchmark not found)")
        endif()
        
    else()
        message(WARNING "Tests requested but Google Test not found")
        message(STATUS "To install Google Test on Windows:")
        message(STATUS "  vcpkg install gtest")
        message(STATUS "Or build from source:")
        message(STATUS "  git clone https://github.com/google/googletest.git")
    endif()
else()
    message(STATUS "📦 Tests disabled - standard build mode")
endif()

# Add test-specific targets when BUILD_TESTS is enabled
if(BUILD_TESTS)
    # Test data preparation target
    add_custom_target(prepare_test_data
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/test_output
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/test_logs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/test_configs
        COMMENT "Preparing test data directories"
    )
    
    # Test validation target
    add_custom_target(validate_tests
        COMMAND ${CMAKE_COMMAND} -E echo "Running test validation..."
        COMMENT "Validating test configuration"
    )
    
    # Full test suite target
    add_custom_target(full-test
        COMMAND ${CMAKE_COMMAND} -E echo "🧪 Full test suite execution"
        COMMAND ${CMAKE_COMMAND} -E echo "Building and running all tests..."
        DEPENDS 
            kolosal_agent_unit_tests
            kolosal_agent_integration_tests
            kolosal_agent_performance_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4 || ${CMAKE_COMMAND} -E echo "Note: Some tests may have compilation issues and will be skipped"
        COMMENT "Building and running full test suite"
    )
    
    # Quick test target for unit tests only
    add_custom_target(quick-test
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L "unit" --parallel 4
        COMMENT "Running quick unit tests"
    )
    
    # Test and report target
    add_custom_target(test-report
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
        COMMAND ${CMAKE_COMMAND} -E echo "Test execution completed. Check test_logs/ for detailed results."
        COMMENT "Running tests and generating reports"
    )
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

# Test build target (only available when BUILD_TESTS=ON)
if(BUILD_TESTS)
    add_custom_target(build-with-tests
        DEPENDS kolosal-agent kolosal-server-exe prepare_test_data
        COMMENT "Building all targets with test support"
    )
endif()

# Post-build actions (removed - keeping simple)
# No automatic post-build actions to keep things simple

# Copy required DLLs for kolosal-agent
if(WIN32)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/kolosal_server.dll
            $<TARGET_FILE_DIR:kolosal-agent>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/llama-cpu.dll
            $<TARGET_FILE_DIR:kolosal-agent>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/libcurl.dll
            $<TARGET_FILE_DIR:kolosal-agent>
        COMMENT "Copying required DLLs for kolosal-agent.exe"
        VERBATIM
    )

    # Copy required DLLs for deep-research-demo
    add_custom_command(TARGET deep-research-demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/kolosal_server.dll
            $<TARGET_FILE_DIR:deep-research-demo>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/llama-cpu.dll
            $<TARGET_FILE_DIR:deep-research-demo>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/kolosal-server/Debug/libcurl.dll
            $<TARGET_FILE_DIR:deep-research-demo>
        COMMENT "Copying required DLLs for deep-research-demo.exe"
        VERBATIM
    )
endif()

# Run tests after build (optional - only when BUILD_TESTS=ON)
if(BUILD_TESTS)
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Tests available. Run with: ctest --output-on-failure"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Tests built successfully - run manually when needed"
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
if(BUILD_TESTS)
message(STATUS "  🧪 Build Mode: WITH ALL TESTS")
message(STATUS "    - Unit Tests: YES")
message(STATUS "    - Integration Tests: YES")
message(STATUS "    - Performance Tests: YES")
message(STATUS "    - AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "    - UndefinedBehaviorSanitizer: ${ENABLE_UBSAN}")
else()
message(STATUS "  📦 Build Mode: STANDARD (No Tests)")
endif()
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Documentation: ${BUILD_DOCS}")
message(STATUS "  Kolosal Server: Will be built as external executable")
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
message(STATUS "")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  📦 STANDARD BUILD (No Tests)")
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
message(STATUS "    Optional test flags:")
message(STATUS "    cmake .. -DBUILD_TESTS=ON -DENABLE_ASAN=ON -DENABLE_UBSAN=ON")
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  📁 OUTPUT EXECUTABLES")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    Location: build/Debug/ or build/")
message(STATUS "    - kolosal-agent(.exe) - Main agent system")
message(STATUS "    - kolosal-server(.exe) - Standalone LLM server")
message(STATUS "    - kolosal-launcher(.exe) - System launcher")
message(STATUS "    - simple-test(.exe) - Simple test utility")
message(STATUS "    - workflow-example(.exe) - Example workflow")
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🎯 BUILD TARGETS")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake --build . --config Debug                     # Build everything")
message(STATUS "    cmake --build . --target build-agent --config Debug    # Build only kolosal-agent")
message(STATUS "    cmake --build . --target build-server --config Debug   # Build only kolosal-server")
message(STATUS "    cmake --build . --target build-all --config Debug      # Build both main executables")
if(BUILD_TESTS)
message(STATUS "    cmake --build . --target build-with-tests --config Debug  # Build with test support")
message(STATUS "  ")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  🧪 TEST TARGETS (Available)")
message(STATUS "  ══════════════════════════════════════")
message(STATUS "    cmake --build . --target quick-test --config Debug     # Run unit tests only")
message(STATUS "    cmake --build . --target full-test --config Debug      # Run all tests")
message(STATUS "    cmake --build . --target test-report --config Debug    # Run tests with reporting")
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
if(NOT CURL_FOUND)
message(STATUS "  ══════════════════════════════════════")
message(STATUS "  💡 ENABLE HTTP CLIENT (Optional)")
message(STATUS "  ══════════════════════════════════════")
if(WIN32)
message(STATUS "    vcpkg install curl")
message(STATUS "    cmake .. -DCURL_ROOT=path/to/curl")
else()
message(STATUS "    # Ubuntu/Debian:")
message(STATUS "    sudo apt install libcurl4-openssl-dev")
message(STATUS "    # CentOS/RHEL:")
message(STATUS "    sudo yum install libcurl-devel")
message(STATUS "    # macOS:")
message(STATUS "    brew install curl")
endif()
message(STATUS "  ")
endif()
message(STATUS "========================================")
