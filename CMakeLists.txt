cmake_minimum_required(VERSION 3.14)

# Set policies for compatibility with newer CMake versions
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # Use <PackageName>_ROOT variables
endif()
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW) # option() honors normal variables
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW) # FetchContent uses DOWNLOAD_EXTRACT_TIMESTAMP
endif()

project(KolosalAgent VERSION 1.0.0 LANGUAGES C CXX)

# Use C++17 for this project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Platform-specific configurations
if (MINGW)
    add_compile_definitions(_WIN32_WINNT=0x602)
endif()

if (WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
endif()

# Options
option(USE_CUDA   "Compile with CUDA support" OFF)
option(USE_VULKAN "Compile with VULKAN support" OFF)
option(USE_METAL  "Compile with Metal support (macOS only)" OFF)
option(USE_MPI    "Compile with MPI support" OFF)
option(DEBUG      "Compile with debugging information" OFF)
option(USE_PODOFO "Compile with PoDoFo PDF support" OFF)
option(BUILD_TESTS "Build test executables" OFF)

# On Apple systems, enable Metal by default unless explicitly disabled
if(APPLE)
    message(STATUS "Apple system detected - enabling Metal acceleration by default")
    set(USE_METAL ON CACHE BOOL "Enable Metal acceleration on Apple systems" FORCE)
endif()

# Define include directories for the agent project
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann)

# Include directories for kolosal-server integration
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/nlohmann)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/yaml-cpp/include)

# Add kolosal-server as subdirectory first so yaml-cpp is available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/CMakeLists.txt")
    message(STATUS "Adding kolosal-server subdirectory...")
    
    # Ensure our options are passed to the submodule
    set(USE_PODOFO ${USE_PODOFO} CACHE BOOL "Compile with PoDoFo PDF support" FORCE)
    set(USE_CUDA ${USE_CUDA} CACHE BOOL "Compile with CUDA support" FORCE)
    set(USE_VULKAN ${USE_VULKAN} CACHE BOOL "Compile with VULKAN support" FORCE)
    set(USE_METAL ${USE_METAL} CACHE BOOL "Compile with Metal support" FORCE)
    set(USE_MPI ${USE_MPI} CACHE BOOL "Compile with MPI support" FORCE)
    
    add_subdirectory(kolosal-server)
else()
    message(FATAL_ERROR "kolosal-server not found. Please initialize the git submodule: git submodule update --init --recursive")
endif()

# Define agent source files
set(AGENT_SOURCES
    src/agent_core.cpp
    src/agent_data.cpp
    src/agent_interfaces.cpp
    src/agent_orchestrator.cpp
    src/agent_roles.cpp
    src/agent_factory.cpp
    src/agent_config_validator.cpp
    src/builtin_functions.cpp
    src/document_agent_service.cpp
    src/document_service_manager.cpp
    src/event_system.cpp
    src/function_manager.cpp
    src/http_client.cpp
    src/job_manager.cpp
    src/kolosal_server_client.cpp
    src/loading_animation.cpp
    src/memory_manager.cpp
    src/message_router.cpp
    src/multi_agent_system.cpp
    src/planning_system.cpp
    src/sequential_workflow.cpp
    src/server_logger_adapter.cpp
    src/tool_registry.cpp
    src/workflow_agent_service.cpp
    src/workflow_engine.cpp
    src/yaml_config.cpp
)

# Create agent library
add_library(kolosal_agent STATIC ${AGENT_SOURCES})

# Ensure agent library has access to all necessary include directories
target_include_directories(kolosal_agent PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/common
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/ggml/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/pugixml/src
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/yaml-cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Link with yaml-cpp
target_link_libraries(kolosal_agent PUBLIC yaml-cpp)

# Link with kolosal-server
target_link_libraries(kolosal_agent PUBLIC kolosal_server)

# Add compile definitions to indicate we're building the agent library
target_compile_definitions(kolosal_agent PUBLIC KOLOSAL_AGENT_BUILD KOLOSAL_AGENT_STATIC)

# Platform-specific link libraries
find_package(Threads REQUIRED)
target_link_libraries(kolosal_agent PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(kolosal_agent PRIVATE ws2_32 wbemuuid wininet)
elseif(APPLE)
    target_link_libraries(kolosal_agent PRIVATE dl pthread)
    
    # Add required macOS frameworks
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    if(COREFOUNDATION_FRAMEWORK)
        target_link_libraries(kolosal_agent PRIVATE ${COREFOUNDATION_FRAMEWORK})
    endif()
else()
    target_link_libraries(kolosal_agent PRIVATE dl rt pthread)
endif()

# Set optimization flags for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
        
        option(ENABLE_NATIVE_OPTIMIZATION "Enable native CPU optimization (-march=native)" OFF)
        if(ENABLE_NATIVE_OPTIMIZATION)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native")
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -mtune=native")
            message(STATUS "Native CPU optimization enabled")
        endif()
    endif()
endif()

# Create the main executable
add_executable(kolosal_agent_exe src/main.cpp)
target_link_libraries(kolosal_agent_exe PRIVATE kolosal_agent)

# For static builds, we need to explicitly link with the inference engine
# since the CompletionParameters::isValid() method is in the DLL
if(USE_CUDA)
    target_link_libraries(kolosal_agent_exe PRIVATE llama-cuda)
elseif(USE_VULKAN)
    target_link_libraries(kolosal_agent_exe PRIVATE llama-vulkan) 
elseif(USE_METAL)
    target_link_libraries(kolosal_agent_exe PRIVATE llama-metal)
else()
    target_link_libraries(kolosal_agent_exe PRIVATE llama-cpu)
endif()

set_target_properties(kolosal_agent_exe PROPERTIES OUTPUT_NAME "kolosal-agent")

# Ensure the executable can find headers
target_include_directories(kolosal_agent_exe PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/inference/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/common
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/llama.cpp/ggml/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/pugixml/src
    ${CMAKE_CURRENT_SOURCE_DIR}/kolosal-server/external/yaml-cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Add compile definitions for the executable  
target_compile_definitions(kolosal_agent_exe PRIVATE KOLOSAL_AGENT_EXE KOLOSAL_AGENT_STATIC)

# Create the legacy launcher executable (still present for backward compatibility)
add_executable(kolosal_launcher_exe src/kolosal_launcher.c)
set_target_properties(kolosal_launcher_exe PROPERTIES OUTPUT_NAME "kolosal-launcher")

# Platform-specific RPATH settings
if(UNIX AND NOT APPLE)
    # Linux-specific RPATH settings
    set_target_properties(kolosal_agent_exe PROPERTIES
        INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib:/usr/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
elseif(APPLE)
    # macOS-specific RPATH settings
    set_target_properties(kolosal_agent_exe PROPERTIES
        INSTALL_RPATH "@executable_path"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        MACOSX_RPATH TRUE
    )
    
    # Additional RPATH entries for macOS
    set_property(TARGET kolosal_agent_exe APPEND PROPERTY INSTALL_RPATH "@executable_path/../lib")
    set_property(TARGET kolosal_agent_exe APPEND PROPERTY INSTALL_RPATH "/usr/local/lib")
    set_property(TARGET kolosal_agent_exe APPEND PROPERTY INSTALL_RPATH "/opt/homebrew/lib")
endif()

# Copy server executable and libraries to agent build directory for easy deployment
if(WIN32)
    # Copy server executable and required DLLs
    add_custom_command(TARGET kolosal_agent_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:kolosal_server_exe>"
        "$<TARGET_FILE_DIR:kolosal_agent_exe>/"
        COMMENT "Copying kolosal-server.exe to agent directory"
    )
    
    # Copy kolosal server DLL
    add_custom_command(TARGET kolosal_agent_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:kolosal_server>"
        "$<TARGET_FILE_DIR:kolosal_agent_exe>/"
        COMMENT "Copying kolosal_server.dll to agent directory"
    )
    
elseif(APPLE OR UNIX)
    # Copy server executable and shared libraries
    add_custom_command(TARGET kolosal_agent_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:kolosal_server_exe>"
        "$<TARGET_FILE_DIR:kolosal_agent_exe>/"
        COMMENT "Copying kolosal-server to agent directory"
    )
    
    # Copy kolosal server shared library
    add_custom_command(TARGET kolosal_agent_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:kolosal_server>"
        "$<TARGET_FILE_DIR:kolosal_agent_exe>/"
        COMMENT "Copying kolosal_server shared library to agent directory"
    )
endif()

# Install rules (optional, for packaging)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    install(TARGETS kolosal_agent_exe
        RUNTIME DESTINATION bin
        COMPONENT Runtime
    )
    
    install(TARGETS kolosal_launcher_exe
        RUNTIME DESTINATION bin
        COMPONENT Runtime
    )
    
    # Install server executable too for complete package
    install(TARGETS kolosal_server_exe
        RUNTIME DESTINATION bin
        COMPONENT Runtime
    )
endif()

# Build tests if enabled
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== Kolosal Agent Configuration Summary ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CUDA Support: ${USE_CUDA}")
message(STATUS "Vulkan Support: ${USE_VULKAN}")
message(STATUS "Metal Support: ${USE_METAL}")
message(STATUS "MPI Support: ${USE_MPI}")
message(STATUS "PoDoFo PDF Support: ${USE_PODOFO}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "===========================================")
message(STATUS "")
