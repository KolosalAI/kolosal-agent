#!/bin/bash

# Test runner for Kolosal Agent System
# Builds and runs the comprehensive test suite for the Kolosal Agent System.
# Supports different test categories and provides detailed reporting.

set -e  # Exit on any error

# Default parameters
TEST_TYPE="all"
BUILD_FIRST=true
VERBOSE_OUTPUT=false
OUTPUT_FILE=""
TIMEOUT_MINUTES=1

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Usage function
show_usage() {
    echo -e "${BLUE}================================================================${NC}"
    echo -e "${BLUE}  Kolosal Agent System Test Runner${NC}"
    echo -e "${BLUE}================================================================${NC}"
    echo ""
    echo -e "${BLUE}Usage: ./run_tests.sh [options]${NC}"
    echo ""
    echo -e "${YELLOW}Test Types:${NC}"
    echo -e "  ${BLUE}all${NC}          - Run all tests (default)"
    echo -e "  ${BLUE}demo${NC}         - Run demo tests (minimal_demo)"
    echo -e "  ${BLUE}quick${NC}        - Run quick unit tests (minimal_demo, model_interface, config_manager, workflow_config)"
    echo -e "  ${BLUE}workflow${NC}     - Run workflow tests (workflow_config, workflow_manager, workflow_orchestrator)"
    echo -e "  ${BLUE}integration${NC}  - Run integration tests (agent_execution, http_server)"
    echo -e "  ${BLUE}retrieval${NC}    - Run retrieval agent tests (retrieval_agent)"
    echo -e "  ${BLUE}deep_research${NC} - Run deep research workflow tests (deep_research)"
    echo -e "  ${BLUE}stress${NC}       - Run stress and error tests (error_scenarios)"
    echo -e "  ${BLUE}<test_name>${NC}  - Run specific test (any individual test name)"
    echo ""
    echo -e "${YELLOW}Options:${NC}"
    echo -e "  ${BLUE}-t, --test-type <type>${NC}     - Test type to run"
    echo -e "  ${BLUE}-b, --build-first <bool>${NC}   - Build tests before running (default: true)"
    echo -e "  ${BLUE}-v, --verbose${NC}              - Enable verbose output"
    echo -e "  ${BLUE}-o, --output-file <file>${NC}   - Save results to JSON file"
    echo -e "  ${BLUE}--timeout <minutes>${NC}        - Timeout for individual tests in minutes (default: 1)"
    echo -e "  ${BLUE}-h, --help${NC}                 - Show this help message"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo -e "  ${BLUE}./run_tests.sh -t all${NC}"
    echo -e "  ${BLUE}./run_tests.sh --test-type quick --verbose${NC}"
    echo -e "  ${BLUE}./run_tests.sh -t workflow -o results.json${NC}"
    echo -e "  ${BLUE}./run_tests.sh -t retrieval --verbose${NC}"
    echo -e "  ${BLUE}./run_tests.sh -t deep_research --verbose${NC}"
    echo -e "  ${BLUE}./run_tests.sh -t model_interface --verbose${NC}"
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -t|--test-type)
                TEST_TYPE="$2"
                shift 2
                ;;
            -b|--build-first)
                if [[ "$2" == "false" || "$2" == "0" ]]; then
                    BUILD_FIRST=false
                else
                    BUILD_FIRST=true
                fi
                shift 2
                ;;
            -v|--verbose)
                VERBOSE_OUTPUT=true
                shift
                ;;
            -o|--output-file)
                OUTPUT_FILE="$2"
                shift 2
                ;;
            --timeout)
                TIMEOUT_MINUTES="$2"
                shift 2
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                echo -e "${RED}Unknown option: $1${NC}"
                show_usage
                exit 1
                ;;
        esac
    done
}

# Color output functions
write_color_output() {
    local message="$1"
    local color="${2:-$NC}"
    echo -e "${color}${message}${NC}"
}

write_header() {
    local title="$1"
    echo ""
    write_color_output "============================================================" "$BLUE"
    write_color_output "  $title" "$BLUE"
    write_color_output "============================================================" "$BLUE"
}

write_section() {
    local title="$1"
    echo ""
    write_color_output "----------------------------------------" "$CYAN"
    write_color_output "  $title" "$CYAN"
    write_color_output "----------------------------------------" "$CYAN"
}

# Constants
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/tests_build"
TEST_RESULTS_DIR="$BUILD_DIR/test_results"

# Check prerequisites
test_prerequisites() {
    write_section "Checking Prerequisites"
    
    # Check if CMake is available
    if command -v cmake >/dev/null 2>&1; then
        write_color_output "✓ CMake is available" "$GREEN"
        if [[ "$VERBOSE_OUTPUT" == "true" ]]; then
            cmake_version=$(cmake --version | head -n 1)
            write_color_output "  Version: $cmake_version" "$BLUE"
        fi
    else
        write_color_output "✗ CMake is not available" "$RED"
        echo "CMake is required to build tests"
        exit 1
    fi
    
    # Check if build tools are available
    if command -v make >/dev/null 2>&1; then
        write_color_output "✓ Make is available" "$GREEN"
    elif command -v ninja >/dev/null 2>&1; then
        write_color_output "✓ Ninja is available" "$GREEN"
    else
        write_color_output "⚠ No build system found (make/ninja)" "$YELLOW"
    fi
    
    # Check if compiler is available
    if command -v gcc >/dev/null 2>&1; then
        write_color_output "✓ GCC is available" "$GREEN"
    elif command -v clang >/dev/null 2>&1; then
        write_color_output "✓ Clang is available" "$GREEN"
    else
        write_color_output "⚠ No C++ compiler found" "$YELLOW"
    fi
    
    # Check if source files exist
    local required_files=(
        "include/agent.hpp"
        "include/agent_manager.hpp"
        "include/agent_config.hpp"
        "src/core/agent.cpp"
        "src/core/agent_manager.cpp"
    )
    
    for file in "${required_files[@]}"; do
        local file_path="$PROJECT_ROOT/$file"
        if [[ -f "$file_path" ]]; then
            write_color_output "✓ Found $file" "$GREEN"
        else
            write_color_output "✗ Missing $file" "$RED"
            echo "Required source file missing: $file"
            exit 1
        fi
    done
}

# Build tests
build_tests() {
    write_section "Building Tests"
    
    # Create build directory
    if [[ -d "$BUILD_DIR" ]]; then
        write_color_output "Cleaning existing build directory..." "$YELLOW"
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    mkdir -p "$TEST_RESULTS_DIR"
    
    # Configure with CMake
    write_color_output "Configuring tests with CMake..." "$BLUE"
    cd "$BUILD_DIR"
    
    local cmake_args=(
        "-S" "$PROJECT_ROOT/tests"
        "-B" "."
        "-DCMAKE_BUILD_TYPE=Debug"
    )
    
    if [[ "$VERBOSE_OUTPUT" == "true" ]]; then
        write_color_output "Running CMake with verbose output..." "$BLUE"
    fi
    
    if ! cmake "${cmake_args[@]}"; then
        echo "CMake configuration failed"
        exit 1
    fi
    
    write_color_output "✓ CMake configuration successful" "$GREEN"
    
    # Build tests
    write_color_output "Building test executables..." "$BLUE"
    
    local build_args=(
        "--build" "."
        "--config" "Debug"
    )
    
    if [[ "$VERBOSE_OUTPUT" == "true" ]]; then
        build_args+=("--verbose")
    fi
    
    if ! cmake "${build_args[@]}"; then
        echo "Build failed"
        exit 1
    fi
    
    write_color_output "✓ Build successful" "$GREEN"
    
    cd "$PROJECT_ROOT"
}

# Get available tests
get_available_tests() {
    local build_dir="$1"
    declare -A available_tests
    
    # Try multiple possible locations for test executables
    local possible_dirs=(
        "$build_dir"
        "$build_dir/Debug"
        "$build_dir/Release"
    )
    
    local debug_dir=""
    for dir in "${possible_dirs[@]}"; do
        if [[ -d "$dir" ]]; then
            local test_files=($(find "$dir" -maxdepth 1 -name "*.exe" -o -name "test_*" -o -name "*_test" -o -name "*test_demo" 2>/dev/null | grep -v "\.exe$" || find "$dir" -maxdepth 1 -executable -type f 2>/dev/null))
            if [[ ${#test_files[@]} -gt 0 ]]; then
                debug_dir="$dir"
                write_color_output "Found test executables in: $dir" "$BLUE"
                break
            fi
        fi
    done
    
    if [[ -z "$debug_dir" ]]; then
        write_color_output "⚠ No test executables found in any expected location" "$YELLOW"
        write_color_output "Searched directories:" "$BLUE"
        for dir in "${possible_dirs[@]}"; do
            if [[ -d "$dir" ]]; then
                write_color_output "  ✓ $dir" "$GREEN"
            else
                write_color_output "  ✗ $dir" "$RED"
            fi
        done
        return 1
    fi
    
    write_color_output "Using test directory: $debug_dir" "$BLUE"
    
    # Look for all executable files that look like tests
    local test_files=($(find "$debug_dir" -maxdepth 1 -executable -type f -name "*test*" 2>/dev/null))
    
    for test_file in "${test_files[@]}"; do
        local test_name=$(basename "$test_file")
        local test_key="$test_name"
        
        # Remove common prefixes and suffixes
        test_key="${test_key#test_}"
        test_key="${test_key%_test}"
        test_key="${test_key%test_demo}"
        test_key="${test_key%.exe}"
        
        # Special cases
        if [[ "$test_name" == *"simple_test_demo"* ]]; then
            test_key="simple_demo"
        elif [[ "$test_name" == *"minimal_test_demo"* ]]; then
            test_key="minimal_demo"
        fi
        
        # Verify the executable is valid
        if [[ -x "$test_file" ]]; then
            available_tests["$test_key"]="$test_file"
            write_color_output "  Found test: $test_key -> $(basename $test_file)" "$GREEN"
        fi
    done
    
    # Also include the predefined mappings for consistency
    local -A predefined_tests=(
        ["simple_demo"]="simple_test_demo"
        ["minimal_demo"]="minimal_test_demo"
        ["agent_execution"]="test_agent_execution"
        ["model_interface"]="test_model_interface"
        ["config_manager"]="test_config_manager"
        ["workflow_config"]="test_workflow_config"
        ["workflow_manager"]="test_workflow_manager"
        ["workflow_orchestrator"]="test_workflow_orchestrator"
        ["http_server"]="test_http_server"
        ["error_scenarios"]="test_error_scenarios"
        ["retrieval_agent"]="test_retrieval_agent"
        ["deep_research"]="test_deep_research"
    )
    
    # Merge discovered tests with predefined ones, checking if they actually exist
    for key in "${!predefined_tests[@]}"; do
        local executable_name="${predefined_tests[$key]}"
        local executable_path="$debug_dir/$executable_name"
        if [[ -x "$executable_path" ]]; then
            # Only add if not already discovered
            if [[ -z "${available_tests[$key]:-}" ]]; then
                available_tests["$key"]="$executable_path"
                write_color_output "  Added predefined test: $key -> $executable_name" "$GREEN"
            fi
        else
            write_color_output "  Missing predefined test: $key -> $executable_name" "$YELLOW"
        fi
    done
    
    if [[ ${#available_tests[@]} -eq 0 ]]; then
        write_color_output "⚠ No valid test executables found" "$YELLOW"
        return 1
    else
        write_color_output "Total tests found: ${#available_tests[@]}" "$GREEN"
    fi
    
    # Export available tests for use in other functions
    for key in "${!available_tests[@]}"; do
        eval "AVAILABLE_TESTS_${key}=\"${available_tests[$key]}\""
    done
    
    # Create a list of all available test keys
    AVAILABLE_TEST_KEYS=($(printf '%s\n' "${!available_tests[@]}" | sort))
    
    return 0
}

# Run a single test executable
run_test_executable() {
    local test_name="$1"
    local executable_path="$2"
    local timeout_minutes="$3"
    
    write_color_output "\nRunning $test_name..." "$MAGENTA"
    
    local start_time=$(date +%s.%3N)
    local success=false
    local duration=0
    local output=""
    local exit_code=0
    local error_msg=""
    
    local output_file="$TEST_RESULTS_DIR/${test_name}.xml"
    local log_file="$TEST_RESULTS_DIR/${test_name}.log"
    
    # Ensure test results directory exists
    mkdir -p "$TEST_RESULTS_DIR"
    
    # Verify executable exists before running
    if [[ ! -x "$executable_path" ]]; then
        write_color_output "✗ $test_name FAILED - Executable not found: $executable_path" "$RED"
        echo "success=false duration=0 error=\"Executable not found\""
        return 1
    fi
    
    # Run test with XML output for detailed results
    local test_args=(
        "--gtest_output=xml:$output_file"
    )
    
    if [[ "$VERBOSE_OUTPUT" == "true" ]]; then
        test_args+=("--gtest_print_time=1")
        test_args+=("--gtest_print_utf8=1")
    fi
    
    # Special handling for error_scenarios and http_server tests to avoid hanging
    if [[ "$test_name" == "error_scenarios" || "$test_name" == "http_server" ]]; then
        # Run without output redirection to avoid buffering issues
        timeout "${timeout_minutes}m" "$executable_path" "${test_args[@]}" 2>&1
        exit_code=$?
        
        local end_time=$(date +%s.%3N)
        duration=$(echo "$end_time - $start_time" | bc -l)
        
        if [[ $exit_code -eq 0 ]]; then
            write_color_output "✓ $test_name PASSED (${duration}s)" "$GREEN"
            success=true
        elif [[ $exit_code -eq 124 ]]; then
            write_color_output "✗ $test_name TIMED OUT (${duration}s)" "$RED"
            error_msg="Test timed out after $timeout_minutes minutes"
        else
            write_color_output "✗ $test_name FAILED (${duration}s)" "$RED"
        fi
        
        echo "success=$success duration=$duration exit_code=$exit_code error=\"$error_msg\""
        return $([ "$success" = "true" ] && echo 0 || echo 1)
    fi
    
    # Adjust timeout for specific tests that need more time
    local actual_timeout_minutes=$timeout_minutes
    if [[ "$test_name" == "error_scenarios" || "$test_name" == "http_server" ]]; then
        actual_timeout_minutes=5  # Increase timeout for stress tests and http server tests
    elif [[ "$test_name" == "deep_research" ]]; then
        actual_timeout_minutes=3  # Deep research tests may need more time
    fi
    
    # Run test with timeout support
    if [[ "$test_name" == *"workflow"* || "$test_name" == *"model_interface"* ]]; then
        # For workflow and model interface tests that may have I/O deadlock issues,
        # use a simpler approach without output redirection
        timeout "${actual_timeout_minutes}m" "$executable_path" "${test_args[@]}" 2>&1
        exit_code=$?
        output=""  # No output captured to prevent deadlock
    else
        # Capture output for other tests
        output=$(timeout "${actual_timeout_minutes}m" "$executable_path" "${test_args[@]}" 2>&1) || exit_code=$?
    fi
    
    local end_time=$(date +%s.%3N)
    duration=$(echo "$end_time - $start_time" | bc -l 2>/dev/null || echo "0")
    
    # Write output to log file
    if [[ -n "$output" ]]; then
        echo "$output" > "$log_file"
    fi
    
    if [[ $exit_code -eq 0 ]]; then
        write_color_output "✓ $test_name PASSED (${duration}s)" "$GREEN"
        success=true
    elif [[ $exit_code -eq 124 ]]; then
        write_color_output "✗ $test_name TIMED OUT (${duration}s)" "$RED"
        error_msg="Test timed out after $actual_timeout_minutes minutes"
    else
        write_color_output "✗ $test_name FAILED (${duration}s)" "$RED"
        if [[ "$VERBOSE_OUTPUT" == "true" && -n "$output" ]]; then
            write_color_output "Error output:" "$RED"
            echo "$output" | grep -E "FAILED|ERROR|FATAL" | while read line; do
                write_color_output "  $line" "$RED"
            done
        fi
    fi
    
    echo "success=$success duration=$duration exit_code=$exit_code error=\"$error_msg\""
    return $([ "$success" = "true" ] && echo 0 || echo 1)
}

# Run tests
run_tests() {
    local test_type="$1"
    
    write_section "Running Tests: $test_type"
    
    # Get available tests
    if ! get_available_tests "$BUILD_DIR"; then
        write_color_output "No test executables found. Build may have failed or tests not compiled." "$RED"
        return 1
    fi
    
    # Define test categories
    declare -A test_categories
    test_categories["demo"]="minimal_demo"
    test_categories["quick"]="minimal_demo model_interface config_manager workflow_config"
    test_categories["workflow"]="workflow_config workflow_manager workflow_orchestrator"
    test_categories["integration"]="agent_execution http_server"
    test_categories["retrieval"]="retrieval_agent"
    test_categories["deep_research"]="deep_research"
    test_categories["stress"]="error_scenarios"
    test_categories["all"]="${AVAILABLE_TEST_KEYS[*]}"
    
    # Determine which tests to run
    local tests_to_run=()
    
    if [[ -n "${test_categories[$test_type]:-}" ]]; then
        # It's a category
        IFS=' ' read -ra requested_tests <<< "${test_categories[$test_type]}"
        # Filter to only include tests that actually exist
        for test in "${requested_tests[@]}"; do
            local var_name="AVAILABLE_TESTS_${test}"
            if [[ -n "${!var_name:-}" ]]; then
                tests_to_run+=("$test")
            fi
        done
        
        # Report missing tests
        local missing_tests=()
        for test in "${requested_tests[@]}"; do
            local var_name="AVAILABLE_TESTS_${test}"
            if [[ -z "${!var_name:-}" ]]; then
                missing_tests+=("$test")
            fi
        done
        
        if [[ ${#missing_tests[@]} -gt 0 ]]; then
            write_color_output "⚠ Some tests in category '$test_type' are not available:" "$YELLOW"
            for missing in "${missing_tests[@]}"; do
                write_color_output "  - $missing" "$YELLOW"
            done
        fi
    else
        # Check if it's an individual test
        local var_name="AVAILABLE_TESTS_${test_type}"
        if [[ -n "${!var_name:-}" ]]; then
            tests_to_run=("$test_type")
        else
            write_color_output "Unknown test type: $test_type" "$RED"
            write_color_output "Available test categories: ${!test_categories[*]}" "$BLUE"
            write_color_output "Available individual tests: ${AVAILABLE_TEST_KEYS[*]}" "$BLUE"
            return 1
        fi
    fi
    
    if [[ ${#tests_to_run[@]} -eq 0 ]]; then
        write_color_output "No tests found to run for type: $test_type" "$YELLOW"
        write_color_output "Available tests: ${AVAILABLE_TEST_KEYS[*]}" "$BLUE"
        return 1
    fi
    
    write_color_output "Tests to run: ${tests_to_run[*]}" "$BLUE"
    write_color_output "Total tests to execute: ${#tests_to_run[@]}" "$BLUE"
    
    if [[ "$VERBOSE_OUTPUT" == "true" ]]; then
        write_color_output "Available test executables:" "$BLUE"
        for key in "${AVAILABLE_TEST_KEYS[@]}"; do
            local var_name="AVAILABLE_TESTS_${key}"
            local executable_path="${!var_name}"
            if [[ -x "$executable_path" ]]; then
                local size=$(stat -f%z "$executable_path" 2>/dev/null || stat -c%s "$executable_path" 2>/dev/null || echo "unknown")
                write_color_output "  ✓ $key -> $(basename $executable_path) ($size bytes)" "$GREEN"
            else
                write_color_output "  ✗ $key -> $(basename $executable_path)" "$RED"
            fi
        done
    fi
    
    # Run tests
    declare -A results
    local total_duration=0
    local passed_tests=0
    local failed_tests=0
    local skipped_tests=0
    
    for test_name in "${tests_to_run[@]}"; do
        local var_name="AVAILABLE_TESTS_${test_name}"
        local executable_path="${!var_name}"
        
        # Check if executable exists and is valid
        if [[ ! -x "$executable_path" ]]; then
            write_color_output "✗ Test executable not found: $executable_path" "$RED"
            ((failed_tests++))
            results["$test_name"]="success=false duration=0 error=\"Executable not found\""
            continue
        fi
        
        # Check if executable is accessible
        if [[ ! -s "$executable_path" ]]; then
            write_color_output "✗ Test executable is empty: $executable_path" "$RED"
            ((failed_tests++))
            results["$test_name"]="success=false duration=0 error=\"Executable is empty\""
            continue
        fi
        
        # Run the test
        write_color_output "Starting test: $test_name" "$CYAN"
        local result=$(run_test_executable "$test_name" "$executable_path" "$TIMEOUT_MINUTES")
        results["$test_name"]="$result"
        
        # Parse result
        local success=$(echo "$result" | grep -o 'success=[^[:space:]]*' | cut -d= -f2)
        local duration=$(echo "$result" | grep -o 'duration=[^[:space:]]*' | cut -d= -f2)
        
        if [[ "$duration" != "0" ]]; then
            total_duration=$(echo "$total_duration + $duration" | bc -l 2>/dev/null || echo "$total_duration")
        fi
        
        if [[ "$success" == "true" ]]; then
            ((passed_tests++))
        else
            ((failed_tests++))
            # Log failure details for debugging
            local error=$(echo "$result" | sed -n 's/.*error="\([^"]*\)".*/\1/p')
            local exit_code=$(echo "$result" | grep -o 'exit_code=[^[:space:]]*' | cut -d= -f2)
            if [[ -n "$error" ]]; then
                write_color_output "  Error details: $error" "$RED"
            fi
            if [[ -n "$exit_code" && "$exit_code" != "0" ]]; then
                write_color_output "  Exit code: $exit_code" "$RED"
            fi
        fi
        
        # Add small delay between tests to prevent resource conflicts
        if [[ ${#tests_to_run[@]} -gt 1 ]]; then
            sleep 0.5
        fi
    done
    
    # Generate summary
    write_header "Test Results Summary"
    
    write_color_output "Test Category: $test_type" "$BLUE"
    write_color_output "Total Tests: ${#tests_to_run[@]}" "$BLUE"
    write_color_output "Passed: $passed_tests" "$GREEN"
    if [[ $failed_tests -eq 0 ]]; then
        write_color_output "Failed: $failed_tests" "$GREEN"
    else
        write_color_output "Failed: $failed_tests" "$RED"
    fi
    if [[ $skipped_tests -gt 0 ]]; then
        write_color_output "Skipped: $skipped_tests" "$YELLOW"
    fi
    write_color_output "Total Duration: ${total_duration}s" "$BLUE"
    
    if [[ $(echo "$total_duration > 0" | bc -l 2>/dev/null) -eq 1 ]]; then
        local avg_duration=$(echo "scale=2; $total_duration / ${#tests_to_run[@]}" | bc -l 2>/dev/null || echo "0")
        write_color_output "Average Duration per Test: ${avg_duration}s" "$BLUE"
    fi
    
    # Detailed results
    write_color_output "\nDetailed Results:" "$BLUE"
    for test_name in "${tests_to_run[@]}"; do
        if [[ -n "${results[$test_name]:-}" ]]; then
            local result="${results[$test_name]}"
            local success=$(echo "$result" | grep -o 'success=[^[:space:]]*' | cut -d= -f2)
            local duration=$(echo "$result" | grep -o 'duration=[^[:space:]]*' | cut -d= -f2)
            
            local status="FAIL"
            local color="$RED"
            if [[ "$success" == "true" ]]; then
                status="PASS"
                color="$GREEN"
            fi
            
            write_color_output "  $test_name: $status (${duration}s)" "$color"
            
            # Show additional error info for failed tests
            if [[ "$success" != "true" && "$VERBOSE_OUTPUT" == "true" ]]; then
                local error=$(echo "$result" | sed -n 's/.*error="\([^"]*\)".*/\1/p')
                local exit_code=$(echo "$result" | grep -o 'exit_code=[^[:space:]]*' | cut -d= -f2)
                if [[ -n "$error" ]]; then
                    write_color_output "    Error: $error" "$RED"
                fi
                if [[ -n "$exit_code" && "$exit_code" != "0" ]]; then
                    write_color_output "    Exit Code: $exit_code" "$RED"
                fi
            fi
        fi
    done
    
    # Save results to file if requested
    if [[ -n "$OUTPUT_FILE" ]]; then
        local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
        local avg_duration=0
        if [[ ${#tests_to_run[@]} -gt 0 ]]; then
            avg_duration=$(echo "scale=2; $total_duration / ${#tests_to_run[@]}" | bc -l 2>/dev/null || echo "0")
        fi
        
        cat > "$OUTPUT_FILE" << EOF
{
  "TestType": "$test_type",
  "Timestamp": "$timestamp",
  "TotalTests": ${#tests_to_run[@]},
  "PassedTests": $passed_tests,
  "FailedTests": $failed_tests,
  "SkippedTests": $skipped_tests,
  "TotalDuration": $total_duration,
  "AverageDuration": $avg_duration,
  "TestsRun": [$(printf '"%s",' "${tests_to_run[@]}" | sed 's/,$//')]
}
EOF
        write_color_output "\nResults saved to: $OUTPUT_FILE" "$BLUE"
    fi
    
    # Return success only if all tests passed
    local success=$([ $failed_tests -eq 0 ] && echo "true" || echo "false")
    if [[ "$success" == "true" ]]; then
        write_color_output "\n🎉 All tests passed successfully!" "$GREEN"
        return 0
    else
        write_color_output "\n❌ $failed_tests test(s) failed" "$RED"
        return 1
    fi
}

# Main execution
main() {
    # Parse command line arguments
    parse_arguments "$@"
    
    write_header "Kolosal Agent System Test Runner"
    
    write_color_output "Test Type: $TEST_TYPE" "$BLUE"
    write_color_output "Build First: $BUILD_FIRST" "$BLUE"
    write_color_output "Verbose: $VERBOSE_OUTPUT" "$BLUE"
    write_color_output "Timeout: $TIMEOUT_MINUTES minutes" "$BLUE"
    if [[ -n "$OUTPUT_FILE" ]]; then
        write_color_output "Output File: $OUTPUT_FILE" "$BLUE"
    fi
    
    # Check prerequisites
    test_prerequisites
    
    # Build tests if requested
    if [[ "$BUILD_FIRST" == "true" ]]; then
        build_tests
    fi
    
    # Run tests
    if run_tests "$TEST_TYPE"; then
        write_header "All Tests Completed Successfully!"
        exit 0
    else
        write_header "Some Tests Failed!"
        exit 1
    fi
}

# Check if bc is available for floating point arithmetic
if ! command -v bc >/dev/null 2>&1; then
    echo "Warning: 'bc' calculator not found. Duration calculations may be inaccurate."
    echo "Install bc with: sudo apt-get install bc (Debian/Ubuntu) or sudo yum install bc (RHEL/CentOS)"
fi

# Run main function with all arguments
main "$@"
