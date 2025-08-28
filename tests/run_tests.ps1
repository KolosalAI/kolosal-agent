#!/usr/bin/env pwsh

<#
.SYNOPSIS
    Test runner for Kolosal Agent System
.DESCRIPTION
    Builds and runs the comprehensive test suite for the Kolosal Agent System.
    Supports different test categories and provides detailed reporting.
.PARAMETER TestType
    Type of tests to run: all, quick, integration, stress, or specific test name
.PARAMETER BuildFirst
    Whether to build tests before running (default: true)
.PARAMETER Verbose
    Enable verbose output
.PARAMETER OutputFile
    File to save test results
.EXAMPLE
    .\run_tests.ps1 -TestType all
.EXAMPLE
    .\run_tests.ps1 -TestType quick -Verbose
.EXAMPLE
    .\run_tests.ps1 -TestType integration -OutputFile test_results.txt
#>

param(
    [Parameter()]
    [ValidateSet("all", "demo", "quick", "integration", "stress", "simple_demo", "agent_execution", "model_interface", "config_manager", "http_server", "error_scenarios")]
    [string]$TestType = "all",
    
    [Parameter()]
    [bool]$BuildFirst = $true,
    
    [Parameter()]
    [switch]$VerboseOutput,
    
    [Parameter()]
    [string]$OutputFile = ""
)

# Set error handling
$ErrorActionPreference = "Stop"

# Constants
$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$PROJECT_ROOT = Split-Path -Parent $SCRIPT_DIR
$BUILD_DIR = Join-Path $PROJECT_ROOT "tests_build"
$TEST_RESULTS_DIR = Join-Path $BUILD_DIR "test_results"

# Colors for output
$Colors = @{
    Green = "Green"
    Red = "Red" 
    Yellow = "Yellow"
    Blue = "Blue"
    Cyan = "Cyan"
    Magenta = "Magenta"
}

function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Write-Header {
    param([string]$Title)
    Write-ColorOutput "`n$('=' * 60)" $Colors.Blue
    Write-ColorOutput "  $Title" $Colors.Blue
    Write-ColorOutput "$('=' * 60)" $Colors.Blue
}

function Write-Section {
    param([string]$Title)
    Write-ColorOutput "`n$('-' * 40)" $Colors.Cyan
    Write-ColorOutput "  $Title" $Colors.Cyan
    Write-ColorOutput "$('-' * 40)" $Colors.Cyan
}

function Test-Prerequisites {
    Write-Section "Checking Prerequisites"
    
    # Check if CMake is available
    try {
        $cmakeVersion = cmake --version 2>$null
        if ($cmakeVersion) {
            Write-ColorOutput "✓ CMake is available" $Colors.Green
            if ($Verbose) {
                Write-ColorOutput "  Version: $($cmakeVersion[0])" $Colors.Blue
            }
        }
    } catch {
        Write-ColorOutput "✗ CMake is not available" $Colors.Red
        throw "CMake is required to build tests"
    }
    
    # Check if build tools are available
    try {
        $msbuildVersion = msbuild -version 2>$null
        if ($msbuildVersion) {
            Write-ColorOutput "✓ MSBuild is available" $Colors.Green
        }
    } catch {
        Write-ColorOutput "⚠ MSBuild not found, will try alternative build tools" $Colors.Yellow
    }
    
    # Check if source files exist
    $requiredFiles = @(
        "include/agent.hpp",
        "include/agent_manager.hpp", 
        "include/agent_config.hpp",
        "src/core/agent.cpp",
        "src/core/agent_manager.cpp"
    )
    
    foreach ($file in $requiredFiles) {
        $filePath = Join-Path $PROJECT_ROOT $file
        if (Test-Path $filePath) {
            Write-ColorOutput "✓ Found $file" $Colors.Green
        } else {
            Write-ColorOutput "✗ Missing $file" $Colors.Red
            throw "Required source file missing: $file"
        }
    }
}

function Build-Tests {
    Write-Section "Building Tests"
    
    # Create build directory
    if (Test-Path $BUILD_DIR) {
        Write-ColorOutput "Cleaning existing build directory..." $Colors.Yellow
        Remove-Item $BUILD_DIR -Recurse -Force
    }
    
    New-Item -ItemType Directory -Path $BUILD_DIR -Force | Out-Null
    New-Item -ItemType Directory -Path $TEST_RESULTS_DIR -Force | Out-Null
    
    # Configure with CMake
    Write-ColorOutput "Configuring tests with CMake..." $Colors.Blue
    Set-Location $BUILD_DIR
    
    try {
        $cmakeArgs = @(
            "-S", (Join-Path $PROJECT_ROOT "tests")
            "-B", "."
            "-DCMAKE_BUILD_TYPE=Debug"
        )
        
        if ($VerboseOutput) {
            $cmakeArgs += "--verbose"
        }
        
        & cmake @cmakeArgs
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        Write-ColorOutput "✓ CMake configuration successful" $Colors.Green
        
        # Build tests
        Write-ColorOutput "Building test executables..." $Colors.Blue
        
        $buildArgs = @(
            "--build", "."
            "--config", "Debug"
        )
        
        if ($VerboseOutput) {
            $buildArgs += "--verbose"
        }
        
        & cmake @buildArgs
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-ColorOutput "✓ Build successful" $Colors.Green
        
    } finally {
        Set-Location $PROJECT_ROOT
    }
}

function Run-TestExecutable {
    param(
        [string]$TestName,
        [string]$ExecutablePath
    )
    
    Write-ColorOutput "`nRunning $TestName..." $Colors.Magenta
    
    $startTime = Get-Date
    
    try {
        $outputFile = Join-Path $TEST_RESULTS_DIR "$TestName.xml"
        $logFile = Join-Path $TEST_RESULTS_DIR "$TestName.log"
        
        # Run test with XML output for detailed results
        $testArgs = @(
            "--gtest_output=xml:$outputFile"
        )
        
        if ($VerboseOutput) {
            $testArgs += "--gtest_print_time=1"
            $testArgs += "--gtest_print_utf8=1"
        }
        
        # Capture both stdout and stderr
        $output = & $ExecutablePath @testArgs 2>&1 | Tee-Object -FilePath $logFile
        $exitCode = $LASTEXITCODE
        
        $endTime = Get-Date
        $duration = ($endTime - $startTime).TotalSeconds
        
        if ($exitCode -eq 0) {
            Write-ColorOutput "✓ $TestName PASSED (${duration}s)" $Colors.Green
            return @{ Success = $true; Duration = $duration; Output = $output }
        } else {
            Write-ColorOutput "✗ $TestName FAILED (${duration}s)" $Colors.Red
            if ($VerboseOutput) {
                Write-ColorOutput "Error output:" $Colors.Red
                $output | Where-Object { $_ -match "FAILED|ERROR|FATAL" } | ForEach-Object {
                    Write-ColorOutput "  $_" $Colors.Red
                }
            }
            return @{ Success = $false; Duration = $duration; Output = $output; ExitCode = $exitCode }
        }
        
    } catch {
        $endTime = Get-Date
        $duration = ($endTime - $startTime).TotalSeconds
        Write-ColorOutput "✗ $TestName CRASHED (${duration}s)" $Colors.Red
        Write-ColorOutput "Error: $($_.Exception.Message)" $Colors.Red
        return @{ Success = $false; Duration = $duration; Error = $_.Exception.Message }
    }
}

function Run-Tests {
    param([string]$TestType)
    
    Write-Section "Running Tests: $TestType"
    
    $testExecutables = @{
        "simple_demo" = "simple_test_demo.exe"
        "minimal_demo" = "minimal_test_demo.exe"
        "agent_execution" = "test_agent_execution.exe"
        "model_interface" = "test_model_interface.exe"
        "config_manager" = "test_config_manager.exe"
        "http_server" = "test_http_server.exe"
        "error_scenarios" = "test_error_scenarios.exe"
    }
    
    $testCategories = @{
        "demo" = @("minimal_demo")
        "quick" = @("minimal_demo", "model_interface", "config_manager")
        "integration" = @("agent_execution", "http_server")
        "stress" = @("error_scenarios")
        "all" = @("simple_demo", "model_interface", "config_manager", "agent_execution", "http_server", "error_scenarios")
    }
    
    # Determine which tests to run
    $testsToRun = @()
    
    if ($testCategories.ContainsKey($TestType)) {
        $testsToRun = $testCategories[$TestType]
    } elseif ($testExecutables.ContainsKey($TestType)) {
        $testsToRun = @($TestType)
    } else {
        Write-ColorOutput "Unknown test type: $TestType" $Colors.Red
        return $false
    }
    
    Write-ColorOutput "Tests to run: $($testsToRun -join ', ')" $Colors.Blue
    
    # Run tests
    $results = @{}
    $totalDuration = 0
    $passedTests = 0
    $failedTests = 0
    
    foreach ($testName in $testsToRun) {
        $executableName = $testExecutables[$testName]
        $executablePath = Join-Path $BUILD_DIR "Debug" $executableName
        
        # Check if executable exists
        if (-not (Test-Path $executablePath)) {
            Write-ColorOutput "✗ Test executable not found: $executablePath" $Colors.Red
            $failedTests++
            continue
        }
        
        $result = Run-TestExecutable -TestName $testName -ExecutablePath $executablePath
        $results[$testName] = $result
        $totalDuration += $result.Duration
        
        if ($result.Success) {
            $passedTests++
        } else {
            $failedTests++
        }
    }
    
    # Generate summary
    Write-Header "Test Results Summary"
    
    Write-ColorOutput "Total Tests: $($testsToRun.Count)" $Colors.Blue
    Write-ColorOutput "Passed: $passedTests" $Colors.Green
    Write-ColorOutput "Failed: $failedTests" $(if ($failedTests -eq 0) { $Colors.Green } else { $Colors.Red })
    Write-ColorOutput "Total Duration: ${totalDuration}s" $Colors.Blue
    
    # Detailed results
    foreach ($testName in $testsToRun) {
        if ($results.ContainsKey($testName)) {
            $result = $results[$testName]
            $status = if ($result.Success) { "PASS" } else { "FAIL" }
            $color = if ($result.Success) { $Colors.Green } else { $Colors.Red }
            Write-ColorOutput "  $testName`: $status (${result.Duration}s)" $color
        }
    }
    
    # Save results to file if requested
    if ($OutputFile) {
        $resultData = @{
            TestType = $TestType
            Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
            TotalTests = $testsToRun.Count
            PassedTests = $passedTests
            FailedTests = $failedTests
            TotalDuration = $totalDuration
            Results = $results
        }
        
        $resultData | ConvertTo-Json -Depth 5 | Out-File -FilePath $OutputFile -Encoding UTF8
        Write-ColorOutput "`nResults saved to: $OutputFile" $Colors.Blue
    }
    
    return ($failedTests -eq 0)
}

function Show-Usage {
    Write-Header "Kolosal Agent System Test Runner"
    
    Write-ColorOutput "Usage: .\run_tests.ps1 [options]" $Colors.Blue
    Write-ColorOutput ""
    Write-ColorOutput "Test Types:" $Colors.Yellow
    Write-ColorOutput "  all          - Run all tests (default)" $Colors.Blue
    Write-ColorOutput "  quick        - Run quick unit tests (model_interface, config_manager)" $Colors.Blue
    Write-ColorOutput "  integration  - Run integration tests (agent_execution, http_server)" $Colors.Blue
    Write-ColorOutput "  stress       - Run stress and error tests (error_scenarios)" $Colors.Blue
    Write-ColorOutput "  <test_name>  - Run specific test (agent_execution, model_interface, etc.)" $Colors.Blue
    Write-ColorOutput ""
    Write-ColorOutput "Options:" $Colors.Yellow
    Write-ColorOutput "  -BuildFirst <bool>   - Build tests before running (default: true)" $Colors.Blue
    Write-ColorOutput "  -VerboseOutput           - Enable verbose output" $Colors.Blue
    Write-ColorOutput "  -OutputFile <file>   - Save results to JSON file" $Colors.Blue
    Write-ColorOutput ""
    Write-ColorOutput "Examples:" $Colors.Yellow
    Write-ColorOutput "  .\run_tests.ps1 -TestType all" $Colors.Blue
    Write-ColorOutput "  .\run_tests.ps1 -TestType quick -VerboseOutput" $Colors.Blue
    Write-ColorOutput "  .\run_tests.ps1 -TestType integration -OutputFile results.json" $Colors.Blue
}

# Main execution
try {
    Write-Header "Kolosal Agent System Test Runner"
    
    Write-ColorOutput "Test Type: $TestType" $Colors.Blue
    Write-ColorOutput "Build First: $BuildFirst" $Colors.Blue
    Write-ColorOutput "Verbose: $VerboseOutput" $Colors.Blue
    if ($OutputFile) {
        Write-ColorOutput "Output File: $OutputFile" $Colors.Blue
    }
    
    # Check prerequisites
    Test-Prerequisites
    
    # Build tests if requested
    if ($BuildFirst) {
        Build-Tests
    }
    
    # Run tests
    $success = Run-Tests -TestType $TestType
    
    if ($success) {
        Write-Header "All Tests Completed Successfully!"
        exit 0
    } else {
        Write-Header "Some Tests Failed!"
        exit 1
    }
    
} catch {
    Write-ColorOutput "`nError: $($_.Exception.Message)" $Colors.Red
    Write-ColorOutput "Stack Trace:" $Colors.Red
    Write-ColorOutput $_.ScriptStackTrace $Colors.Red
    exit 1
}
