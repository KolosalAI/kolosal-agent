cmake_minimum_required(VERSION 3.14)

# Project configuration
project(KolosalAgentSystem 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Multi-Agent System"
)

# C++ Standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Platform detection
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(APPLE)
    add_definitions(-DPLATFORM_MACOS)
else()
    add_definitions(-DPLATFORM_LINUX)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/MDd /Od /Zi)
    else()
        add_compile_options(/MD /O2)
    endif()
else()
    add_compile_options(-Wall -Wextra)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# nlohmann/json - Use local copy
set(NLOHMANN_JSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann")
if(EXISTS "${NLOHMANN_JSON_DIR}/json.hpp")
    include_directories(${NLOHMANN_JSON_DIR})
    message(STATUS "Using nlohmann/json from: ${NLOHMANN_JSON_DIR}")
else()
    message(FATAL_ERROR "nlohmann/json not found at ${NLOHMANN_JSON_DIR}")
endif()

# Source files
set(CORE_SOURCES
    src/core/agent.cpp
    src/core/agent_manager.cpp
    src/core/main.cpp
)

set(API_SOURCES
    src/api/http_server.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${API_SOURCES}
)

# Create executable
add_executable(kolosal-agent-system ${ALL_SOURCES})

# Set target properties
set_target_properties(kolosal-agent-system PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "kolosal-agent-system"
)

# Link libraries
target_link_libraries(kolosal-agent-system PRIVATE Threads::Threads)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(kolosal-agent-system PRIVATE
        ws2_32
        rpcrt4
    )
elseif(APPLE)
    target_link_libraries(kolosal-agent-system PRIVATE
        "-framework Foundation"
        "-framework CoreFoundation"
    )
else()
    # Linux - try to find uuid library
    find_library(UUID_LIBRARY uuid)
    if(UUID_LIBRARY)
        target_link_libraries(kolosal-agent-system PRIVATE ${UUID_LIBRARY})
    endif()
    
    target_link_libraries(kolosal-agent-system PRIVATE dl rt)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS kolosal-agent-system
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Create configuration file
set(CONFIG_CONTENT "# Kolosal Agent System Configuration
system:
  name: \"Kolosal Agent System\"
  version: \"1.0.0\"
  host: \"127.0.0.1\"
  port: 8080

agents:
  - name: \"Assistant\"
    capabilities: [\"chat\", \"analysis\"]
    auto_start: true
  
  - name: \"Analyzer\" 
    capabilities: [\"analysis\", \"data_processing\"]
    auto_start: true

functions:
  chat:
    description: \"Basic chat functionality\"
  analyze:
    description: \"Text analysis functionality\"
  echo:
    description: \"Echo function for testing\"
  status:
    description: \"Agent status information\"
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.yaml" "${CONFIG_CONTENT}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config.yaml"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/kolosal-agent-system
)

# Summary
message(STATUS "=== Kolosal Agent System Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target: kolosal-agent-system")
message(STATUS "==========================================")

# Create example usage file
set(EXAMPLE_CONTENT "#!/bin/bash
# Kolosal Agent System - Quick Start Examples

HOST=\"127.0.0.1\"
PORT=\"8080\"
BASE_URL=\"http://\${HOST}:\${PORT}\"

echo \"=== Kolosal Agent System Examples ===\"
echo \"Server: \${BASE_URL}\"
echo \"\"

echo \"1. List all agents:\"
echo \"curl \${BASE_URL}/agents\"
echo \"\"

echo \"2. Get system status:\"
echo \"curl \${BASE_URL}/status\"
echo \"\"

echo \"3. Chat with assistant (replace AGENT_ID):\"
echo \"curl -X POST \${BASE_URL}/agents/AGENT_ID/execute \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"function\\\": \\\"chat\\\", \\\"params\\\": {\\\"message\\\": \\\"Hello!\\\"}}'\"
echo \"\"

echo \"4. Analyze text (replace AGENT_ID):\"
echo \"curl -X POST \${BASE_URL}/agents/AGENT_ID/execute \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"function\\\": \\\"analyze\\\", \\\"params\\\": {\\\"text\\\": \\\"This is a test\\\"}}'\"
echo \"\"

echo \"5. Create new agent:\"
echo \"curl -X POST \${BASE_URL}/agents \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"name\\\": \\\"MyAgent\\\", \\\"capabilities\\\": [\\\"chat\\\"]}'\"
echo \"\"
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/examples.sh" "${EXAMPLE_CONTENT}")

if(UNIX)
    # Make executable on Unix systems
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/examples.sh" 
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                     GROUP_READ GROUP_EXECUTE 
                     WORLD_READ WORLD_EXECUTE)
endif()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/examples.sh"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
