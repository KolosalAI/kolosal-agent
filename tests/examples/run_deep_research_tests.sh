#!/bin/bash

# Deep Research Agent Test Runner
# Comprehensive test execution script for the Deep Research Agent

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default configuration
BUILD_DIR="build"
TEST_TYPE="unit"
VERBOSE=false
COVERAGE=false
SERVER_URL="http://localhost:8080"

# Function to print colored output
print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to print usage
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type TYPE       Test type: unit, integration, performance, all (default: unit)"
    echo "  -b, --build-dir DIR   Build directory (default: build)"
    echo "  -v, --verbose         Enable verbose output"
    echo "  -c, --coverage        Enable test coverage reporting"
    echo "  -s, --server URL      Server URL for integration tests (default: http://localhost:8080)"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 -t unit                    # Run unit tests only"
    echo "  $0 -t integration -v          # Run integration tests with verbose output"
    echo "  $0 -t all -c                  # Run all tests with coverage"
    echo "  $0 -t performance -s http://localhost:9090  # Run performance tests with custom server"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            TEST_TYPE="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -c|--coverage)
            COVERAGE=true
            shift
            ;;
        -s|--server)
            SERVER_URL="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# Validate test type
case $TEST_TYPE in
    unit|integration|performance|all)
        ;;
    *)
        print_colored $RED "Error: Invalid test type '$TEST_TYPE'"
        print_usage
        exit 1
        ;;
esac

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    print_colored $RED "Error: Build directory '$BUILD_DIR' does not exist"
    exit 1
fi

# Change to build directory
cd "$BUILD_DIR"

print_colored $BLUE "=== Deep Research Agent Test Runner ==="
print_colored $BLUE "Test Type: $TEST_TYPE"
print_colored $BLUE "Build Directory: $BUILD_DIR"
print_colored $BLUE "Verbose: $VERBOSE"
print_colored $BLUE "Coverage: $COVERAGE"
print_colored $BLUE "Server URL: $SERVER_URL"
print_colored $BLUE "========================================"

# Set environment variables for integration tests
if [ "$TEST_TYPE" = "integration" ] || [ "$TEST_TYPE" = "all" ]; then
    export KOLOSAL_INTEGRATION_TESTS=1
    export KOLOSAL_SERVER_URL="$SERVER_URL"
    print_colored $YELLOW "Integration tests enabled with server: $SERVER_URL"
fi

# Set environment variables for performance tests
if [ "$TEST_TYPE" = "performance" ] || [ "$TEST_TYPE" = "all" ]; then
    export KOLOSAL_PERFORMANCE_TESTS=1
    print_colored $YELLOW "Performance tests enabled"
fi

# Function to run unit tests
run_unit_tests() {
    print_colored $GREEN "\n=== Running Unit Tests ==="
    
    if [ ! -f "./kolosal_agent_unit_tests" ] && [ ! -f "./kolosal_agent_unit_tests.exe" ]; then
        print_colored $RED "Error: Unit test executable not found"
        return 1
    fi
    
    local test_cmd="./kolosal_agent_unit_tests"
    if [ -f "./kolosal_agent_unit_tests.exe" ]; then
        test_cmd="./kolosal_agent_unit_tests.exe"
    fi
    
    if [ "$VERBOSE" = true ]; then
        test_cmd="$test_cmd --gtest_output=xml:unit_test_results.xml"
    fi
    
    print_colored $BLUE "Command: $test_cmd"
    
    if $test_cmd; then
        print_colored $GREEN "✓ Unit tests passed"
        return 0
    else
        print_colored $RED "✗ Unit tests failed"
        return 1
    fi
}

# Function to run integration tests
run_integration_tests() {
    print_colored $GREEN "\n=== Running Integration Tests ==="
    
    if [ ! -f "./kolosal_agent_integration_tests" ] && [ ! -f "./kolosal_agent_integration_tests.exe" ]; then
        print_colored $YELLOW "Warning: Integration test executable not found, skipping"
        return 0
    fi
    
    local test_cmd="./kolosal_agent_integration_tests"
    if [ -f "./kolosal_agent_integration_tests.exe" ]; then
        test_cmd="./kolosal_agent_integration_tests.exe"
    fi
    
    if [ "$VERBOSE" = true ]; then
        test_cmd="$test_cmd --gtest_output=xml:integration_test_results.xml"
    fi
    
    print_colored $BLUE "Command: $test_cmd"
    
    # Check server connectivity before running integration tests
    print_colored $YELLOW "Checking server connectivity at $SERVER_URL..."
    if ! curl -s --max-time 5 "$SERVER_URL/health" > /dev/null 2>&1; then
        print_colored $YELLOW "Warning: Server not reachable at $SERVER_URL"
        print_colored $YELLOW "Integration tests may skip server-dependent tests"
    fi
    
    if $test_cmd; then
        print_colored $GREEN "✓ Integration tests passed"
        return 0
    else
        print_colored $RED "✗ Integration tests failed"
        return 1
    fi
}

# Function to run performance tests
run_performance_tests() {
    print_colored $GREEN "\n=== Running Performance Tests ==="
    
    if [ ! -f "./kolosal_agent_performance_tests" ] && [ ! -f "./kolosal_agent_performance_tests.exe" ]; then
        print_colored $YELLOW "Warning: Performance test executable not found, skipping"
        return 0
    fi
    
    local test_cmd="./kolosal_agent_performance_tests"
    if [ -f "./kolosal_agent_performance_tests.exe" ]; then
        test_cmd="./kolosal_agent_performance_tests.exe"
    fi
    
    if [ "$VERBOSE" = true ]; then
        test_cmd="$test_cmd --gtest_output=xml:performance_test_results.xml"
    fi
    
    print_colored $BLUE "Command: $test_cmd"
    print_colored $YELLOW "Note: Performance tests may take several minutes to complete"
    
    if $test_cmd; then
        print_colored $GREEN "✓ Performance tests passed"
        return 0
    else
        print_colored $RED "✗ Performance tests failed"
        return 1
    fi
}

# Function to generate coverage report
generate_coverage() {
    if [ "$COVERAGE" = true ]; then
        print_colored $GREEN "\n=== Generating Coverage Report ==="
        
        if command -v lcov > /dev/null 2>&1; then
            print_colored $BLUE "Capturing coverage data..."
            lcov --capture --directory . --output-file coverage.info
            
            print_colored $BLUE "Filtering coverage data..."
            lcov --remove coverage.info '/usr/*' '*/external/*' '*/tests/*' --output-file coverage_filtered.info
            
            print_colored $BLUE "Generating HTML report..."
            genhtml coverage_filtered.info --output-directory coverage_report
            
            print_colored $GREEN "✓ Coverage report generated in coverage_report/"
        else
            print_colored $YELLOW "Warning: lcov not found, skipping coverage report"
        fi
    fi
}

# Main execution
main() {
    local exit_code=0
    
    case $TEST_TYPE in
        unit)
            run_unit_tests || exit_code=1
            ;;
        integration)
            run_integration_tests || exit_code=1
            ;;
        performance)
            run_performance_tests || exit_code=1
            ;;
        all)
            run_unit_tests || exit_code=1
            run_integration_tests || exit_code=1
            run_performance_tests || exit_code=1
            ;;
    esac
    
    generate_coverage
    
    print_colored $BLUE "\n========================================"
    if [ $exit_code -eq 0 ]; then
        print_colored $GREEN "✓ All requested tests completed successfully"
    else
        print_colored $RED "✗ Some tests failed"
    fi
    print_colored $BLUE "========================================"
    
    return $exit_code
}

# Run main function
main
exit $?
