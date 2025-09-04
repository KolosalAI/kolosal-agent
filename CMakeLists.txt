cmake_minimum_required(VERSION 3.14)

# Project configuration
project(KolosalAgent 
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "Kolosal Agent - Multi-Agent System with LLM Integration"
)

# Include UCM for runtime library management (if available)
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/ucm.cmake")
    include(${CMAKE_SOURCE_DIR}/cmake/ucm.cmake)
    # Static link the runtime libraries for better distribution
    ucm_set_runtime(STATIC)
endif()

# C++ Standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Set output directories for consistent build output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_KOLOSAL_SERVER "Build the Kolosal Server component" ON)
option(BUILD_WITH_RETRIEVAL "Enable retrieval functionality" ON)
option(ENABLE_NATIVE_OPTIMIZATION "Enable native CPU optimization (-march=native)" OFF)
option(BUILD_EXAMPLES "Build example applications" OFF)

# Ensure BUILD_WITH_RETRIEVAL is compatible with BUILD_KOLOSAL_SERVER
if(BUILD_WITH_RETRIEVAL AND NOT BUILD_KOLOSAL_SERVER)
    message(WARNING "BUILD_WITH_RETRIEVAL requires BUILD_KOLOSAL_SERVER - enabling it automatically")
    set(BUILD_KOLOSAL_SERVER ON)
endif()

# macOS specific options
if(APPLE)
    option(BUILD_UNIVERSAL2 "Build universal (arm64;x86_64) binaries" OFF)
    option(ENABLE_CODESIGN "Enable code signing on macOS" OFF)
    set(CODESIGN_IDENTITY "" CACHE STRING "Code signing identity")
    
    if(BUILD_UNIVERSAL2)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Architectures for macOS build" FORCE)
    endif()
    
    # Set minimum macOS version
    if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET OR CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS minimum deployment target" FORCE)
        else()
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "macOS minimum deployment target" FORCE)
        endif()
    endif()
endif()

# Compiler flags and platform detection
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(APPLE)
    add_definitions(-DPLATFORM_MACOS)
    add_definitions(-D__APPLE__)
    add_definitions(-D__MACOS__)
else()
    add_definitions(-DPLATFORM_LINUX)
    add_definitions(-D_GNU_SOURCE)
    add_definitions(-D__LINUX__)
endif()

# Compiler-specific flags
if(MSVC)
    add_compile_options(/W4 /utf-8 /FS)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    # Fix for Debug builds with kolosal-server: avoid RTC/optimization conflicts
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
        # Remove default /RTC1 flag to avoid conflicts with subprojects using /O2
        string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        if(ENABLE_NATIVE_OPTIMIZATION)
            add_compile_options(-march=native -mtune=native -O3)
        else()
            add_compile_options(-O3)
        endif()
    endif()
endif()

# Find required packages
find_package(Threads REQUIRED)

# Platform-specific package requirements
if(APPLE)
    find_package(OpenSSL)
    find_package(ZLIB)
elseif(UNIX AND NOT APPLE)
    find_package(OpenSSL)
    find_package(ZLIB)
    find_package(PkgConfig QUIET)
endif()

# Include directories
include_directories(include)

# External libraries setup
add_subdirectory(external)

set(NLOHMANN_JSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann")
if(EXISTS "${NLOHMANN_JSON_DIR}/json.hpp")
    include_directories(${NLOHMANN_JSON_DIR})
    message(STATUS "Using nlohmann/json from: ${NLOHMANN_JSON_DIR}")
else()
    message(FATAL_ERROR "nlohmann/json not found at ${NLOHMANN_JSON_DIR}")
endif()

# YAML-CPP setup
set(YAML_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/yaml-cpp")
if(EXISTS "${YAML_CPP_DIR}/CMakeLists.txt")
    add_subdirectory(${YAML_CPP_DIR})
    message(STATUS "Using yaml-cpp from: ${YAML_CPP_DIR}")
else()
    message(FATAL_ERROR "yaml-cpp not found at ${YAML_CPP_DIR}")
endif()

# Source files organization
set(CORE_SOURCES
    src/core/agent.cpp
    src/core/agent_manager.cpp
    src/core/agent_config.cpp
    src/core/model_interface.cpp
    src/core/client.cpp
    src/core/logger.cpp
    src/core/server_launcher.cpp
    src/core/main.cpp
    src/core/retrieval_enhanced.cpp
)

set(API_SOURCES
    src/api/server_http.cpp
    src/api/metrics.cpp
    src/api/service_async.cpp
    src/api/logger_advanced.cpp
)

set(WORKFLOW_SOURCES
    src/workflows/workflow_manager.cpp
    src/workflows/workflow_types.cpp
)

set(TOOL_SOURCES
    src/functions/retrieval.cpp
    src/functions/research.cpp
)

# Add retrieval manager if kolosal-server is built
if(BUILD_KOLOSAL_SERVER)
    list(APPEND CORE_SOURCES src/core/retrieval_manager.cpp)
endif()

# Collect all header files for IDE support
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${API_SOURCES}
    ${WORKFLOW_SOURCES}
    ${TOOL_SOURCES}
)

# Create main executable
add_executable(kolosal-agent ${ALL_SOURCES} ${HEADERS})

# Create an alias for the main target to ensure it's always built
add_custom_target(kolosal-agent-system ALL DEPENDS kolosal-agent)

# Set target properties
set_target_properties(kolosal-agent PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "kolosal-agent"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Add /FS flag specifically to the kolosal-agent target for MSVC
if(MSVC)
    target_compile_options(kolosal-agent PRIVATE /FS)
endif()

# Define BUILD_WITH_RETRIEVAL if the option is enabled
if(BUILD_WITH_RETRIEVAL)
    target_compile_definitions(kolosal-agent PRIVATE BUILD_WITH_RETRIEVAL)
endif()

# Link basic libraries
target_link_libraries(kolosal-agent PRIVATE Threads::Threads yaml-cpp)

# Platform-specific libraries and configurations
if(WIN32)
    target_link_libraries(kolosal-agent PRIVATE
        ws2_32
        rpcrt4
        winmm
        bcrypt
    )
    
    # Ensure kolosal-agent.exe is also copied to the main build directory for convenience
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:kolosal-agent>"
        "${CMAKE_BINARY_DIR}/kolosal-agent.exe"
        COMMENT "Copying kolosal-agent.exe to main build directory"
    )
    
elseif(APPLE)
    target_link_libraries(kolosal-agent PRIVATE
        "-framework Foundation"
        "-framework CoreFoundation"
        "-framework Security"
        pthread
        dl
    )
    
    # Link found system libraries
    if(OpenSSL_FOUND)
        target_link_libraries(kolosal-agent PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
    
    if(ZLIB_FOUND)
        target_link_libraries(kolosal-agent PRIVATE ZLIB::ZLIB)
    endif()
    
    # Set RPATH for runtime library search
    set_target_properties(kolosal-agent PROPERTIES
        INSTALL_RPATH "@executable_path;@executable_path/../lib;/usr/local/lib;/opt/homebrew/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    
else()
    # Linux
    target_link_libraries(kolosal-agent PRIVATE
        pthread
        dl
        rt
    )
    
    # Find and link UUID library if available
    find_library(UUID_LIBRARY uuid)
    if(UUID_LIBRARY)
        target_link_libraries(kolosal-agent PRIVATE ${UUID_LIBRARY})
    endif()
    
    # Link found system libraries
    if(OpenSSL_FOUND)
        target_link_libraries(kolosal-agent PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
    
    if(ZLIB_FOUND)
        target_link_libraries(kolosal-agent PRIVATE ZLIB::ZLIB)
    endif()
    
    # Link filesystem library for older GCC versions
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(kolosal-agent PRIVATE stdc++fs)
    endif()
    
    # Set RPATH for runtime library search
    set_target_properties(kolosal-agent PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN:/usr/lib/kolosal:/usr/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

# Add kolosal-server subdirectory if enabled
if(BUILD_KOLOSAL_SERVER)
    message(STATUS "Building Kolosal Server component")
    
    # Ensure kolosal-server builds its executable in the same output directory
    set(KOLOSAL_SERVER_OUTPUT_DIR "${CMAKE_BINARY_DIR}" CACHE STRING "Output directory for kolosal-server")
    
    add_subdirectory(kolosal-server)
    
    # Add kolosal-server include directories for retrieval functionality
    target_include_directories(kolosal-agent PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/include
    )
    
    # Link kolosal-server to main executable for retrieval capabilities
    target_link_libraries(kolosal-agent PRIVATE kolosal_server)
    
    # Always define BUILD_WITH_RETRIEVAL when kolosal-server is built
    target_compile_definitions(kolosal-agent PRIVATE BUILD_WITH_RETRIEVAL)
    
    # Copy kolosal-server files to output directory for all platforms
    add_custom_command(TARGET kolosal-agent POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:kolosal_server>"
        "$<TARGET_FILE_DIR:kolosal-agent>/"
        COMMENT "Copying kolosal_server library to output directory"
    )
    
    # Always copy kolosal-server executable to the same directory as kolosal-agent
    if(TARGET kolosal_server_exe)
        add_custom_command(TARGET kolosal-agent POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:kolosal_server_exe>"
            "$<TARGET_FILE_DIR:kolosal-agent>/"
            COMMENT "Copying kolosal-server executable to output directory"
        )
    endif()
    
    # For platforms where the executable might have different names or locations
    if(WIN32)
        # Windows specific handling for DLLs and executables
        if(TARGET kolosal_server_exe)
            add_custom_command(TARGET kolosal-agent POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE_DIR:kolosal_server_exe>/kolosal-server.exe"
                "$<TARGET_FILE_DIR:kolosal-agent>/kolosal-server.exe"
                COMMENT "Ensuring kolosal-server.exe is in the same directory as kolosal-agent"
                VERBATIM
            )
        endif()
    elseif(UNIX)
        # Unix/Linux/macOS specific handling
        if(TARGET kolosal_server_exe)
            add_custom_command(TARGET kolosal-agent POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE_DIR:kolosal_server_exe>/kolosal-server"
                "$<TARGET_FILE_DIR:kolosal-agent>/kolosal-server"
                COMMENT "Ensuring kolosal-server binary is in the same directory as kolosal-agent"
                VERBATIM
            )
        endif()
    endif()
endif()

# Examples support
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
include(GNUInstallDirs)

# Install main executable
install(TARGETS kolosal-agent
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Runtime
)

# Install kolosal-server targets if built
if(BUILD_KOLOSAL_SERVER)
    if(TARGET kolosal_server_exe)
        install(TARGETS kolosal_server_exe
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
        )
    endif()
    
    if(TARGET kolosal_server)
        install(TARGETS kolosal_server
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Runtime
        )
    endif()
endif()

# Install headers for development
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kolosal-agent
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Create and install configuration files
set(AGENT_CONFIG_CONTENT "# Kolosal Agent System Configuration
system:
  name: \"Kolosal Agent System\"
  version: \"${PROJECT_VERSION}\"
  host: \"127.0.0.1\"
  port: 8080
  log_level: \"info\"
  max_concurrent_requests: 100

# System instruction/prompt that will be used for all agents
system_instruction: |
  You are a helpful AI assistant that is part of the Kolosal Agent System. You have been designed to assist users with various tasks including:
  
  - Answering questions and providing information
  - Analyzing text and data
  - Helping with research and problem-solving
  - Providing explanations and tutorials
  - Assisting with creative tasks
  
  You should always:
  - Be helpful, accurate, and honest
  - Admit when you don't know something
  - Provide clear and well-structured responses
  - Be respectful and professional
  - Follow ethical guidelines
  
  Your responses should be informative and helpful while being concise when appropriate.

# Default agents to create on startup
agents:
  - name: \"Assistant\"
    capabilities: [\"chat\", \"analysis\", \"reasoning\"]
    auto_start: true
    model: \"default\"
    system_prompt: |
      You are an AI assistant specialized in general conversation and help. 
      You excel at answering questions, providing explanations, and helping users with various tasks.
      Be friendly, helpful, and informative in your responses.
  
  - name: \"Analyzer\" 
    capabilities: [\"analysis\", \"data_processing\", \"summarization\"]
    auto_start: true
    model: \"default\"
    system_prompt: |
      You are an AI analyst specialized in text and data analysis.
      Your role is to examine, process, and summarize information effectively.
      Provide detailed analysis with clear insights and actionable conclusions.

  - name: \"Researcher\"
    capabilities: [\"research\", \"web_search\", \"fact_checking\"]
    auto_start: false
    model: \"default\"
    system_prompt: |
      You are an AI researcher specialized in information gathering and fact-checking.
      Your role is to help users find accurate information and verify facts.
      Always cite sources when possible and present information objectively.

# Available functions that agents can execute
functions:
  chat:
    description: \"Interactive chat functionality\"
    timeout: 30000
    parameters:
      - name: \"message\"
        type: \"string\"
        required: true
        description: \"Message to send to the agent\"
  
  analyze:
    description: \"Text and data analysis functionality\"
    timeout: 60000
    parameters:
      - name: \"text\"
        type: \"string\"
        required: true
        description: \"Text to analyze\"
      - name: \"analysis_type\"
        type: \"string\"
        required: false
        description: \"Type of analysis to perform (sentiment, summary, keywords, etc.)\"
  
  research:
    description: \"Research and information gathering\"
    timeout: 120000
    parameters:
      - name: \"query\"
        type: \"string\"
        required: true
        description: \"Research query or topic\"
      - name: \"depth\"
        type: \"string\"
        required: false
        description: \"Research depth (basic, detailed, comprehensive)\"
  
  echo:
    description: \"Echo function for testing\"
    timeout: 5000
    parameters:
      - name: \"data\"
        type: \"any\"
        required: false
        description: \"Data to echo back\"
  
  status:
    description: \"Agent status information\"
    timeout: 5000
    parameters: []

# Performance and resource configuration
performance:
  max_memory_usage: \"2GB\"
  cache_size: \"512MB\"
  worker_threads: 4
  request_timeout: 30000
  max_request_size: \"10MB\"

# Logging configuration
logging:
  level: \"info\"
  file: \"agent_system.log\"
  max_file_size: \"100MB\"
  max_files: 10
  console_output: true

# Security configuration
security:
  enable_cors: true
  allowed_origins: [\"http://localhost:3000\", \"http://127.0.0.1:3000\"]
  max_request_rate: 100  # requests per minute per IP
  enable_auth: false
  api_key: \"\"  # Set this if enable_auth is true
")

set(KOLOSAL_SERVER_CONFIG_CONTENT "# Kolosal Server Configuration
# This configuration is specifically for the kolosal-server component
server:
  name: \"Kolosal Server\"
  version: \"${PROJECT_VERSION}\"
  host: \"127.0.0.1\"
  port: 3000
  log_level: \"info\"
  max_workers: 4

# Model configuration
models:
  default:
    path: \"models/qwen2.5-0.5b-instruct-q4_k_m.gguf\"
    context_length: 4096
    threads: 4
    gpu_layers: 0
    
  embedding:
    path: \"models/all-MiniLM-L6-v2-bf16-q4_k.gguf\"
    dimensions: 384

# Inference settings
inference:
  temperature: 0.7
  top_p: 0.9
  top_k: 40
  repeat_penalty: 1.1
  max_tokens: 2048
  stream: true

# API configuration
api:
  cors_enabled: true
  cors_origins: [\"http://localhost:8080\", \"http://127.0.0.1:8080\"]
  rate_limit: 60  # requests per minute
  timeout: 120000  # 2 minutes

# Retrieval settings (if enabled)
retrieval:
  enabled: false
  vector_store: \"faiss\"
  chunk_size: 512
  chunk_overlap: 50
  similarity_threshold: 0.7

# Performance tuning
performance:
  cache_size: \"1GB\"
  batch_size: 32
  prefill_cache: true
  
# Logging
logging:
  level: \"info\"
  file: \"kolosal_server.log\"
  max_file_size: \"50MB\"
  max_files: 5
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/agent.yaml" "${AGENT_CONFIG_CONTENT}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.yaml" "${KOLOSAL_SERVER_CONFIG_CONTENT}")

# Install configuration files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/agent.yaml"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/kolosal-agent
    COMPONENT Runtime
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config.yaml"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/kolosal-agent
    COMPONENT Runtime
)

# Install documentation if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT Documentation
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT Documentation
    )
endif()

# CPack configuration for packaging
include(CPack)

# Package configuration
set(CPACK_PACKAGE_NAME "kolosal-agent")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Kolosal Agent - Multi-Agent LLM System")
set(CPACK_PACKAGE_DESCRIPTION "A sophisticated multi-agent system for Large Language Models with advanced capabilities including chat, analysis, research, and retrieval.")
set(CPACK_PACKAGE_VENDOR "Kolosal Project")
set(CPACK_PACKAGE_CONTACT "dev@kolosal.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/kolosalai/kolosal-agent")

# Components
set(CPACK_COMPONENTS_ALL Runtime Development Documentation)
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Main executable and runtime libraries")
set(CPACK_COMPONENT_RUNTIME_REQUIRED TRUE)
set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "Development")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Headers and development files")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Documentation and examples")

# Platform-specific package settings
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Kolosal Agent")
    set(CPACK_NSIS_PACKAGE_NAME "Kolosal Agent")
    set(CPACK_NSIS_HELP_LINK "https://github.com/kolosalai/kolosal-agent")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/kolosalai/kolosal-agent")
    set(CPACK_NSIS_CONTACT "dev@kolosal.com")
    set(CPACK_NSIS_MODIFY_PATH ON)
    
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "Kolosal Agent")
    set(CPACK_DMG_FORMAT "UDBZ")
    
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kolosal Project <dev@kolosal.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgcc-s1, libstdc++6")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

# Summary
message(STATUS "=== Kolosal Agent Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Default Target: kolosal-agent-system (builds kolosal-agent.exe)")
if(BUILD_KOLOSAL_SERVER)
    message(STATUS "Kolosal Server: Enabled")
else()
    message(STATUS "Kolosal Server: Disabled")
endif()
if(BUILD_WITH_RETRIEVAL)
    message(STATUS "Retrieval Support: Enabled")
else()
    message(STATUS "Retrieval Support: Disabled")
endif()
if(BUILD_EXAMPLES)
    message(STATUS "Examples: Enabled")
else()
    message(STATUS "Examples: Disabled")
endif()
if(ENABLE_NATIVE_OPTIMIZATION)
    message(STATUS "Native Optimization: Enabled")
else()
    message(STATUS "Native Optimization: Disabled")
endif()
message(STATUS "Installation Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "======================================")

# Development helpers
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build - enabling additional development features")
    add_compile_definitions(DEBUG_BUILD)
endif()

# Create example usage script
set(EXAMPLE_CONTENT "#!/bin/bash
# Kolosal Agent - Quick Start Examples

HOST=\"127.0.0.1\"
PORT=\"8080\"
BASE_URL=\"http://\${HOST}:\${PORT}\"

echo \"=== Kolosal Agent Examples ===\"
echo \"Agent System Server: \${BASE_URL}\"
echo \"Configuration: agent.yaml (agent system)\"
echo \"\"

echo \"1. Get system status:\"
echo \"curl \${BASE_URL}/status\"
echo \"\"

echo \"2. List all agents:\"
echo \"curl \${BASE_URL}/agents\"
echo \"\"

echo \"3. Get agent details (replace AGENT_ID):\"
echo \"curl \${BASE_URL}/agents/AGENT_ID\"
echo \"\"

echo \"4. Chat with assistant:\"
echo \"curl -X POST \${BASE_URL}/agents/assistant/execute \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"function\\\": \\\"chat\\\", \\\"params\\\": {\\\"message\\\": \\\"Hello, how can you help me?\\\"}}'\"
echo \"\"

echo \"5. Analyze text:\"
echo \"curl -X POST \${BASE_URL}/agents/analyzer/execute \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"function\\\": \\\"analyze\\\", \\\"params\\\": {\\\"text\\\": \\\"This is a sample text for analysis\\\"}}'\"
echo \"\"

echo \"6. Create new agent:\"
echo \"curl -X POST \${BASE_URL}/agents \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"name\\\": \\\"CustomAgent\\\", \\\"capabilities\\\": [\\\"chat\\\", \\\"research\\\"]}'\"
echo \"\"

echo \"7. Health check:\"
echo \"curl \${BASE_URL}/health\"
echo \"\"

echo \"=== Kolosal Server Examples (if built with retrieval support) ===\"
echo \"Note: Kolosal Server typically runs on port 3000\"
echo \"Configuration: config.yaml (kolosal server)\"
echo \"SERVER_URL=\\\"http://\${HOST}:3000\\\"\"
echo \"\"

echo \"8. Server status:\"
echo \"curl \${SERVER_URL}/status\"
echo \"\"

echo \"9. List models:\"
echo \"curl \${SERVER_URL}/v1/models\"
echo \"\"

echo \"10. Text completion:\"
echo \"curl -X POST \${SERVER_URL}/v1/completions \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"model\\\": \\\"default\\\", \\\"prompt\\\": \\\"Explain machine learning\\\", \\\"max_tokens\\\": 100}'\"
echo \"\"

echo \"11. Chat completion:\"
echo \"curl -X POST \${SERVER_URL}/v1/chat/completions \\\\\"
echo \"  -H \\\"Content-Type: application/json\\\" \\\\\"
echo \"  -d '{\\\"model\\\": \\\"default\\\", \\\"messages\\\": [{\\\"role\\\": \\\"user\\\", \\\"content\\\": \\\"What is AI?\\\"}]}'\"
echo \"\"
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/examples.sh" "${EXAMPLE_CONTENT}")

# Make executable on Unix systems
if(UNIX)
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/examples.sh" 
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                     GROUP_READ GROUP_EXECUTE 
                     WORLD_READ WORLD_EXECUTE)
endif()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/examples.sh"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT Documentation
)
