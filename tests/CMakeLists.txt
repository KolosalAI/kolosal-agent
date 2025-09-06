# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # Download and build Google Test if not found
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7188a6eac525897bb4b7.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Test executable for security improvements
add_executable(security_improvements_test
    security_improvements_test.cpp
    security_test.cpp
)

# Link test libraries
target_link_libraries(security_improvements_test
    PRIVATE
    gtest_main
    gtest
    # Link against the same libraries as main executable
    Threads::Threads
    yaml-cpp
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Include directories for tests
target_include_directories(security_improvements_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/nlohmann
)

# Add test sources from main project (for testing)
target_sources(security_improvements_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/http_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/model_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/path_validator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/logger.cpp
)

# Platform-specific libraries for tests
if(WIN32)
    target_link_libraries(security_improvements_test PRIVATE ws2_32 rpcrt4 winmm bcrypt)
elseif(APPLE)
    target_link_libraries(security_improvements_test PRIVATE
        "-framework Foundation"
        "-framework CoreFoundation"
        "-framework Security"
        pthread dl
    )
else()
    target_link_libraries(security_improvements_test PRIVATE pthread dl rt)
    
    # Find and link UUID library if available
    find_library(UUID_LIBRARY uuid)
    if(UUID_LIBRARY)
        target_link_libraries(security_improvements_test PRIVATE ${UUID_LIBRARY})
    endif()
endif()

# Register tests with CTest
add_test(NAME SecurityImprovementsTest COMMAND security_improvements_test)

# Set test properties
set_tests_properties(SecurityImprovementsTest PROPERTIES
    TIMEOUT 60
    LABELS "security;unit"
)
