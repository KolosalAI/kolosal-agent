# External dependencies CMakeLists.txt

# Only add subdirectories that are NOT already handled by the main CMakeLists.txt

# GoogleTest (when building tests) - only if not already added
if((BUILD_TESTS OR BUILD_TEST_EXECUTABLE) AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt" AND NOT TARGET gtest)
    # Force GoogleTest to use dynamic runtime to match the main project
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(googletest)
endif()

# CURL (if it has CMakeLists.txt)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/curl/CMakeLists.txt" AND NOT TARGET curl)
    add_subdirectory(curl)
endif()

# nlohmann/json is header-only, no CMakeLists.txt needed
# yaml-cpp is handled by the main CMakeLists.txt
