# External dependencies CMakeLists.txt

# Only add subdirectories that are NOT already handled by the main CMakeLists.txt

# CURL (if it has CMakeLists.txt)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/curl/CMakeLists.txt" AND NOT TARGET curl)
    add_subdirectory(curl)
endif()

# nlohmann/json is header-only, no CMakeLists.txt needed
# yaml-cpp is handled by the main CMakeLists.txt
