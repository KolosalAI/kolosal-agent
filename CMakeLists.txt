cmake_minimum_required(VERSION 3.20)

project(KolosalAgent 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Kolosal Agent - Multi-Agent System with LLM Integration"
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_KOLOSAL_SERVER "Build the Kolosal Server component" ON)
option(BUILD_EXAMPLES "Build example applications" OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Find packages
find_package(Threads REQUIRED)

# Local CURL setup (use bundled version)
set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/curl")
set(CURL_INCLUDE_DIR "${CURL_ROOT}/include")
if(WIN32)
    set(CURL_LIBRARY "${CURL_ROOT}/lib/libcurl.lib")
else()
    set(CURL_LIBRARY "${CURL_ROOT}/lib/libcurl.a")
endif()

# Check if CURL files exist
if(EXISTS "${CURL_INCLUDE_DIR}/curl/curl.h" AND EXISTS "${CURL_LIBRARY}")
    set(CURL_FOUND TRUE)
    message(STATUS "Using bundled CURL: ${CURL_LIBRARY}")
else()
    find_package(CURL REQUIRED)
endif()

find_package(OpenSSL)

# External dependencies
add_subdirectory(external)

# Include directories
include_directories(include)
include_directories(SYSTEM external/nlohmann)

# JSON configuration
add_compile_definitions(JSON_THROW_USER=throw JSON_NOEXCEPTION=0)

# YAML-CPP setup
if(NOT TARGET yaml-cpp)
    add_subdirectory(external/yaml-cpp)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Main executable
add_executable(kolosal-agent ${SOURCES} ${HEADERS})

# Create default target that builds kolosal-agent
add_custom_target(kolosal-agent-system ALL DEPENDS kolosal-agent)

# Target properties
set_target_properties(kolosal-agent PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Link libraries
target_link_libraries(kolosal-agent PRIVATE 
    Threads::Threads 
    yaml-cpp
)

# Link CURL
if(CURL_FOUND)
    if(TARGET CURL::libcurl)
        target_link_libraries(kolosal-agent PRIVATE CURL::libcurl)
    else()
        target_link_libraries(kolosal-agent PRIVATE ${CURL_LIBRARY})
        target_include_directories(kolosal-agent PRIVATE ${CURL_INCLUDE_DIR})
    endif()
endif()

# Optional OpenSSL
if(OpenSSL_FOUND)
    target_link_libraries(kolosal-agent PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(kolosal-agent PRIVATE HAVE_OPENSSL)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(kolosal-agent PRIVATE ws2_32 rpcrt4 winmm bcrypt)
elseif(APPLE)
    target_link_libraries(kolosal-agent PRIVATE 
        "-framework Foundation" 
        "-framework CoreFoundation" 
        "-framework Security"
    )
else()
    target_link_libraries(kolosal-agent PRIVATE pthread dl rt)
endif()

# Kolosal Server integration
if(BUILD_KOLOSAL_SERVER)
    add_subdirectory(kolosal-server)
    target_link_libraries(kolosal-agent PRIVATE kolosal_server)
    target_compile_definitions(kolosal-agent PRIVATE BUILD_WITH_RETRIEVAL)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Testing
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS kolosal-agent
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_KOLOSAL_SERVER AND TARGET kolosal_server_exe)
    install(TARGETS kolosal_server_exe
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kolosal-agent
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install sample configuration files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/configs/agent.yaml")
    install(FILES configs/agent.yaml
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/kolosal-agent
        RENAME agent.yaml.example
    )
endif()

# Summary
message(STATUS "=== Kolosal Agent Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Kolosal Server: ${BUILD_KOLOSAL_SERVER}")
message(STATUS "Examples: ${BUILD_EXAMPLES}")
message(STATUS "==========================================")
