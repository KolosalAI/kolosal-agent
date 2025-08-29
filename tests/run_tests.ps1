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
.PARAMETER TimeoutMinutes
    Timeout in minutes for individual test execution (default: 10)
.EXAMPLE
    .\run_tests.ps1 -TestType all
.EXAMPLE
    .\run_tests.ps1 -TestType quick -Verbose
.EXAMPLE
    .\run_tests.ps1 -TestType integration -OutputFile test_results.txt
.EXAMPLE
    .\run_tests.ps1 -TestType retrieval -TimeoutMinutes 5
#>

param(
    [Parameter()]
    [ValidateSet("all", "demo", "quick", "workflow", "integration", "stress", "retrieval", "simple_demo", "minimal_demo", "agent_execution", "model_interface", "config_manager", "workflow_config", "workflow_manager", "workflow_orchestrator", "http_server", "error_scenarios", "retrieval_agent")]
    [string]$TestType = "all",
    
    [Parameter()]
    [bool]$BuildFirst = $true,
    
    [Parameter()]
    [switch]$VerboseOutput,
    
    [Parameter()]
    [string]$OutputFile = "",
    
    [Parameter()]
    [int]$TimeoutMinutes = 1
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
            Write-ColorOutput "Running CMake with verbose output..." $Colors.Blue
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
        [string]$ExecutablePath,
        [int]$TimeoutMinutes = 1
    )
    
    Write-ColorOutput "`nRunning $TestName..." $Colors.Magenta
    
    $startTime = Get-Date
    
    try {
        $outputFile = Join-Path $TEST_RESULTS_DIR "$TestName.xml"
        $logFile = Join-Path $TEST_RESULTS_DIR "$TestName.log"
        
        # Ensure test results directory exists
        if (-not (Test-Path $TEST_RESULTS_DIR)) {
            New-Item -ItemType Directory -Path $TEST_RESULTS_DIR -Force | Out-Null
        }
        
        # Verify executable exists before running
        if (-not (Test-Path $ExecutablePath)) {
            Write-ColorOutput "✗ $TestName FAILED - Executable not found: $ExecutablePath" $Colors.Red
            return @{ Success = $false; Duration = 0; Error = "Executable not found" }
        }
        
        # Run test with XML output for detailed results
        $testArgs = @(
            "--gtest_output=xml:$outputFile"
        )
        
        if ($VerboseOutput) {
            $testArgs += "--gtest_print_time=1"
            $testArgs += "--gtest_print_utf8=1"
        }
        
        # Special handling for error_scenarios and http_server tests to avoid hanging
        if ($TestName -eq "error_scenarios" -or $TestName -eq "http_server") {
            # Run without output redirection to avoid buffering issues
            $process = Start-Process -FilePath $ExecutablePath -ArgumentList $testArgs -Wait -PassThru -NoNewWindow
            $exitCode = $process.ExitCode
            
            $endTime = Get-Date
            $duration = ($endTime - $startTime).TotalSeconds
            
            if ($exitCode -eq 0) {
                Write-ColorOutput "✓ $TestName PASSED (${duration}s)" $Colors.Green
                return @{ Success = $true; Duration = $duration; Output = @() }
            } else {
                Write-ColorOutput "✗ $TestName FAILED (${duration}s)" $Colors.Red
                return @{ Success = $false; Duration = $duration; Output = @(); ExitCode = $exitCode }
            }
        }
        
        # Adjust timeout for specific tests that need more time
        $actualTimeoutMinutes = $TimeoutMinutes
        if ($TestName -eq "error_scenarios" -or $TestName -eq "http_server") {
            $actualTimeoutMinutes = 5  # Increase timeout for stress tests and http server tests
        }
        
        # Run test with timeout support using Start-Process
        # For workflow and model interface tests that may have I/O deadlock issues,
        # use a simpler approach without output redirection
        if ($TestName -match "workflow|model_interface") {
            $process = Start-Process -FilePath $ExecutablePath -ArgumentList $testArgs -Wait -PassThru -NoNewWindow
            $exitCode = $process.ExitCode
            $output = @()  # No output captured to prevent deadlock
            $errorOutput = ""
        } else {
            $processStartInfo = New-Object System.Diagnostics.ProcessStartInfo
            $processStartInfo.FileName = $ExecutablePath
            $processStartInfo.Arguments = $testArgs -join " "
            $processStartInfo.RedirectStandardOutput = $true
            $processStartInfo.RedirectStandardError = $true
            $processStartInfo.UseShellExecute = $false
            $processStartInfo.CreateNoWindow = $true
            
            $process = New-Object System.Diagnostics.Process
            $process.StartInfo = $processStartInfo
            
            # Start process and wait with timeout
            $process.Start() | Out-Null
            $timeoutMs = $actualTimeoutMinutes * 60 * 1000
            
            # Monitor progress with periodic updates
            $checkIntervalMs = 2000  # Check every 2 seconds
            $totalWaitedMs = 0
            
            while ($totalWaitedMs -lt $timeoutMs) {
                $processExited = $process.WaitForExit($checkIntervalMs)
                if ($processExited) {
                    break
                }
                
                $totalWaitedMs += $checkIntervalMs
                $elapsedSeconds = [math]::Round($totalWaitedMs / 1000, 1)
                
                # Show progress for long-running tests
                if (($TestName -eq "error_scenarios" -or $TestName -eq "http_server") -and ($totalWaitedMs % 5000) -eq 0) {
                    Write-ColorOutput "  Still running... (${elapsedSeconds}s elapsed)" $Colors.Yellow
                }
            }
            
            if (-not $processExited) {
                # Kill process if it times out
                try {
                    $process.Kill()
                    $process.WaitForExit(5000) # Wait up to 5 seconds for process to die
                } catch {
                    Write-ColorOutput "Warning: Could not kill timed-out process" $Colors.Yellow
                }
                
                $endTime = Get-Date
                $duration = ($endTime - $startTime).TotalSeconds
                Write-ColorOutput "✗ $TestName TIMED OUT (${duration}s)" $Colors.Red
                return @{ Success = $false; Duration = $duration; Error = "Test timed out after $actualTimeoutMinutes minutes" }
            }
            
            # Get output and exit code
            $output = $process.StandardOutput.ReadToEnd()
            $errorOutput = $process.StandardError.ReadToEnd()
            $exitCode = $process.ExitCode
        }
        
        # Combine outputs
        $combinedOutput = @()
        if ($output) { $combinedOutput += $output.Split("`n") }
        if ($errorOutput) { $combinedOutput += $errorOutput.Split("`n") }
        
        # Write output to log file
        $combinedOutput | Out-File -FilePath $logFile -Encoding UTF8
        
        $endTime = Get-Date
        $duration = ($endTime - $startTime).TotalSeconds
        
        if ($exitCode -eq 0) {
            Write-ColorOutput "✓ $TestName PASSED (${duration}s)" $Colors.Green
            return @{ Success = $true; Duration = $duration; Output = $combinedOutput }
        } else {
            Write-ColorOutput "✗ $TestName FAILED (${duration}s)" $Colors.Red
            if ($VerboseOutput) {
                Write-ColorOutput "Error output:" $Colors.Red
                $combinedOutput | Where-Object { $_ -match "FAILED|ERROR|FATAL" } | ForEach-Object {
                    Write-ColorOutput "  $_" $Colors.Red
                }
            }
            return @{ Success = $false; Duration = $duration; Output = $combinedOutput; ExitCode = $exitCode }
        }
        
    } catch {
        $endTime = Get-Date
        $duration = ($endTime - $startTime).TotalSeconds
        Write-ColorOutput "✗ $TestName CRASHED (${duration}s)" $Colors.Red
        Write-ColorOutput "Error: $($_.Exception.Message)" $Colors.Red
        return @{ Success = $false; Duration = $duration; Error = $_.Exception.Message }
    } finally {
        # Clean up process if it still exists
        if ($process -and !$process.HasExited) {
            try {
                $process.Kill()
                $process.WaitForExit(1000)
            } catch {
                # Ignore cleanup errors
            }
        }
        if ($process) {
            $process.Dispose()
        }
    }
}

function Get-AvailableTests {
    param([string]$BuildDir)
    
    $availableTests = @{}
    
    # Try multiple possible locations for test executables
    $possibleDirs = @(
        (Join-Path $BuildDir "Debug"),
        (Join-Path $BuildDir "Release"),
        $BuildDir,
        (Join-Path $BuildDir "x64\Debug"),
        (Join-Path $BuildDir "x64\Release")
    )
    
    $debugDir = $null
    foreach ($dir in $possibleDirs) {
        if (Test-Path $dir) {
            $testFiles = Get-ChildItem -Path $dir -Name "*.exe" -ErrorAction SilentlyContinue
            if ($testFiles.Count -gt 0) {
                $debugDir = $dir
                Write-ColorOutput "Found test executables in: $dir" $Colors.Blue
                break
            }
        }
    }
    
    if (-not $debugDir) {
        Write-ColorOutput "⚠ No test executables found in any expected location" $Colors.Yellow
        Write-ColorOutput "Searched directories:" $Colors.Blue
        foreach ($dir in $possibleDirs) {
            $exists = Test-Path $dir
            Write-ColorOutput "  $($exists ? '✓' : '✗') $dir" $(if ($exists) { $Colors.Green } else { $Colors.Red })
        }
        return $availableTests
    }
    
    Write-ColorOutput "Using test directory: $debugDir" $Colors.Blue
    
    # Look for all .exe files that look like tests
    $testPatterns = @("test_*.exe", "*_test.exe", "*test_demo.exe")
    
    foreach ($pattern in $testPatterns) {
        $testFiles = Get-ChildItem -Path $debugDir -Name $pattern -ErrorAction SilentlyContinue
        foreach ($testFile in $testFiles) {
            $testName = [System.IO.Path]::GetFileNameWithoutExtension($testFile)
            $testKey = $testName -replace "^test_", "" -replace "_test$", "" -replace "test_demo$", "demo"
            $fullPath = Join-Path $debugDir $testFile
            
            # Verify the executable is valid
            if (Test-Path $fullPath) {
                $availableTests[$testKey] = @{
                    ExecutableName = $testFile
                    FullPath = $fullPath
                }
                Write-ColorOutput "  Found test: $testKey -> $testFile" $Colors.Green
            }
        }
    }
    
    # Also include the predefined mappings for consistency
    $predefinedTests = @{
        "simple_demo" = "simple_test_demo.exe"
        "minimal_demo" = "minimal_test_demo.exe"
        "agent_execution" = "test_agent_execution.exe"
        "model_interface" = "test_model_interface.exe"
        "config_manager" = "test_config_manager.exe"
        "workflow_config" = "test_workflow_config.exe"
        "workflow_manager" = "test_workflow_manager.exe"
        "workflow_orchestrator" = "test_workflow_orchestrator.exe"
        "http_server" = "test_http_server.exe"
        "error_scenarios" = "test_error_scenarios.exe"
        "retrieval_agent" = "test_retrieval_agent.exe"
    }
    
    # Merge discovered tests with predefined ones, checking if they actually exist
    foreach ($key in $predefinedTests.Keys) {
        $executablePath = Join-Path $debugDir $predefinedTests[$key]
        if (Test-Path $executablePath) {
            # Only add if not already discovered
            if (-not $availableTests.ContainsKey($key)) {
                $availableTests[$key] = @{
                    ExecutableName = $predefinedTests[$key]
                    FullPath = $executablePath
                }
                Write-ColorOutput "  Added predefined test: $key -> $($predefinedTests[$key])" $Colors.Green
            }
        } else {
            Write-ColorOutput "  Missing predefined test: $key -> $($predefinedTests[$key])" $Colors.Yellow
        }
    }
    
    if ($availableTests.Count -eq 0) {
        Write-ColorOutput "⚠ No valid test executables found" $Colors.Yellow
    } else {
        Write-ColorOutput "Total tests found: $($availableTests.Count)" $Colors.Green
    }
    
    return $availableTests
}

function Run-Tests {
    param([string]$TestType)
    
    Write-Section "Running Tests: $TestType"
    
    # Dynamically discover available tests
    $testExecutables = Get-AvailableTests -BuildDir $BUILD_DIR
    
    if ($testExecutables.Count -eq 0) {
        Write-ColorOutput "No test executables found. Build may have failed or tests not compiled." $Colors.Red
        return $false
    }
    
    # Get all available test names for the "all" category
    $allAvailableTests = $testExecutables.Keys | Sort-Object
    
    $testCategories = @{
        "demo" = @("minimal_demo")
        "quick" = @("minimal_demo", "model_interface", "config_manager", "workflow_config")
        "workflow" = @("workflow_config", "workflow_manager", "workflow_orchestrator")
        "integration" = @("agent_execution", "http_server")
        "retrieval" = @("retrieval_agent")
        "stress" = @("error_scenarios")
        "all" = $allAvailableTests
    }
    
    # Determine which tests to run
    $testsToRun = @()
    
    if ($testCategories.ContainsKey($TestType)) {
        $requestedTests = $testCategories[$TestType]
        # Filter to only include tests that actually exist
        $testsToRun = $requestedTests | Where-Object { $testExecutables.ContainsKey($_) }
        
        # Report missing tests
        $missingTests = $requestedTests | Where-Object { -not $testExecutables.ContainsKey($_) }
        if ($missingTests.Count -gt 0) {
            Write-ColorOutput "⚠ Some tests in category '$TestType' are not available:" $Colors.Yellow
            foreach ($missing in $missingTests) {
                Write-ColorOutput "  - $missing" $Colors.Yellow
            }
        }
    } elseif ($testExecutables.ContainsKey($TestType)) {
        $testsToRun = @($TestType)
    } else {
        Write-ColorOutput "Unknown test type: $TestType" $Colors.Red
        Write-ColorOutput "Available test categories: $($testCategories.Keys -join ', ')" $Colors.Blue
        Write-ColorOutput "Available individual tests: $($testExecutables.Keys -join ', ')" $Colors.Blue
        return $false
    }
    
    if ($testsToRun.Count -eq 0) {
        Write-ColorOutput "No tests found to run for type: $TestType" $Colors.Yellow
        Write-ColorOutput "Available tests: $($testExecutables.Keys -join ', ')" $Colors.Blue
        return $false
    }
    
    Write-ColorOutput "Tests to run: $($testsToRun -join ', ')" $Colors.Blue
    Write-ColorOutput "Total tests to execute: $($testsToRun.Count)" $Colors.Blue
    
    if ($VerboseOutput) {
        Write-ColorOutput "Available test executables:" $Colors.Blue
        foreach ($key in $testExecutables.Keys | Sort-Object) {
            $testInfo = $testExecutables[$key]
            $exists = Test-Path $testInfo.FullPath
            $status = if ($exists) { "✓" } else { "✗" }
            $sizeInfo = ""
            if ($exists) {
                $size = (Get-Item $testInfo.FullPath).Length
                $sizeInfo = " ($('{0:N0}' -f $size) bytes)"
            }
            Write-ColorOutput "  $status $key -> $($testInfo.ExecutableName)$sizeInfo" $(if ($exists) { $Colors.Green } else { $Colors.Red })
        }
    }
    
    # Run tests
    $results = @{}
    $totalDuration = 0
    $passedTests = 0
    $failedTests = 0
    $skippedTests = 0
    
    foreach ($testName in $testsToRun) {
        $testInfo = $testExecutables[$testName]
        $executablePath = $testInfo.FullPath
        
        # Check if executable exists and is valid
        if (-not (Test-Path $executablePath)) {
            Write-ColorOutput "✗ Test executable not found: $executablePath" $Colors.Red
            $failedTests++
            $results[$testName] = @{ Success = $false; Duration = 0; Error = "Executable not found" }
            continue
        }
        
        # Check if executable is accessible
        try {
            $fileInfo = Get-Item $executablePath -ErrorAction Stop
            if ($fileInfo.Length -eq 0) {
                Write-ColorOutput "✗ Test executable is empty: $executablePath" $Colors.Red
                $failedTests++
                $results[$testName] = @{ Success = $false; Duration = 0; Error = "Executable is empty" }
                continue
            }
        } catch {
            Write-ColorOutput "✗ Cannot access test executable: $executablePath" $Colors.Red
            $failedTests++
            $results[$testName] = @{ Success = $false; Duration = 0; Error = "Cannot access executable: $($_.Exception.Message)" }
            continue
        }
        
        # Run the test
        Write-ColorOutput "Starting test: $testName" $Colors.Cyan
        $result = Run-TestExecutable -TestName $testName -ExecutablePath $executablePath -TimeoutMinutes $TimeoutMinutes
        $results[$testName] = $result
        $totalDuration += $result.Duration
        
        if ($result.Success) {
            $passedTests++
        } else {
            $failedTests++
            # Log failure details for debugging
            if ($result.Error) {
                Write-ColorOutput "  Error details: $($result.Error)" $Colors.Red
            }
            if ($result.ExitCode) {
                Write-ColorOutput "  Exit code: $($result.ExitCode)" $Colors.Red
            }
        }
        
        # Add small delay between tests to prevent resource conflicts
        if ($testsToRun.Count -gt 1) {
            Start-Sleep -Milliseconds 500
        }
    }
    
    # Generate summary
    Write-Header "Test Results Summary"
    
    Write-ColorOutput "Test Category: $TestType" $Colors.Blue
    Write-ColorOutput "Total Tests: $($testsToRun.Count)" $Colors.Blue
    Write-ColorOutput "Passed: $passedTests" $Colors.Green
    Write-ColorOutput "Failed: $failedTests" $(if ($failedTests -eq 0) { $Colors.Green } else { $Colors.Red })
    if ($skippedTests -gt 0) {
        Write-ColorOutput "Skipped: $skippedTests" $Colors.Yellow
    }
    Write-ColorOutput "Total Duration: ${totalDuration}s" $Colors.Blue
    
    if ($totalDuration -gt 0) {
        $avgDuration = $totalDuration / $testsToRun.Count
        Write-ColorOutput "Average Duration per Test: $($avgDuration.ToString('F2'))s" $Colors.Blue
    }
    
    # Detailed results
    Write-ColorOutput "`nDetailed Results:" $Colors.Blue
    foreach ($testName in $testsToRun) {
        if ($results.ContainsKey($testName)) {
            $result = $results[$testName]
            $status = if ($result.Success) { "PASS" } else { "FAIL" }
            $color = if ($result.Success) { $Colors.Green } else { $Colors.Red }
            $durationStr = "{0:F2}" -f $result.Duration
            Write-ColorOutput "  $testName`: $status (${durationStr}s)" $color
            
            # Show additional error info for failed tests
            if (-not $result.Success -and $VerboseOutput) {
                if ($result.Error) {
                    Write-ColorOutput "    Error: $($result.Error)" $Colors.Red
                }
                if ($result.ExitCode) {
                    Write-ColorOutput "    Exit Code: $($result.ExitCode)" $Colors.Red
                }
            }
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
            SkippedTests = $skippedTests
            TotalDuration = $totalDuration
            AverageDuration = if ($testsToRun.Count -gt 0) { $totalDuration / $testsToRun.Count } else { 0 }
            Results = $results
            TestsRun = $testsToRun
            AvailableTests = $testExecutables.Keys
        }
        
        try {
            $resultData | ConvertTo-Json -Depth 5 | Out-File -FilePath $OutputFile -Encoding UTF8
            Write-ColorOutput "`nResults saved to: $OutputFile" $Colors.Blue
        } catch {
            Write-ColorOutput "`nFailed to save results to: $OutputFile" $Colors.Red
            Write-ColorOutput "Error: $($_.Exception.Message)" $Colors.Red
        }
    }
    
    # Return success only if all tests passed
    $success = ($failedTests -eq 0)
    if ($success) {
        Write-ColorOutput "`n🎉 All tests passed successfully!" $Colors.Green
    } else {
        Write-ColorOutput "`n❌ $failedTests test(s) failed" $Colors.Red
    }
    
    return $success
}

function Show-Usage {
    Write-Header "Kolosal Agent System Test Runner"
    
    Write-ColorOutput "Usage: .\run_tests.ps1 [options]" $Colors.Blue
    Write-ColorOutput ""
    Write-ColorOutput "Test Types:" $Colors.Yellow
    Write-ColorOutput "  all          - Run all tests (default)" $Colors.Blue
    Write-ColorOutput "  demo         - Run demo tests (minimal_demo)" $Colors.Blue
    Write-ColorOutput "  quick        - Run quick unit tests (minimal_demo, model_interface, config_manager, workflow_config)" $Colors.Blue
    Write-ColorOutput "  workflow     - Run workflow tests (workflow_config, workflow_manager, workflow_orchestrator)" $Colors.Blue
    Write-ColorOutput "  integration  - Run integration tests (agent_execution, http_server)" $Colors.Blue
    Write-ColorOutput "  retrieval    - Run retrieval agent tests (retrieval_agent)" $Colors.Blue
    Write-ColorOutput "  stress       - Run stress and error tests (error_scenarios)" $Colors.Blue
    Write-ColorOutput "  <test_name>  - Run specific test (any individual test name)" $Colors.Blue
    Write-ColorOutput ""
    Write-ColorOutput "Options:" $Colors.Yellow
    Write-ColorOutput "  -BuildFirst <bool>     - Build tests before running (default: true)" $Colors.Blue
    Write-ColorOutput "  -VerboseOutput         - Enable verbose output" $Colors.Blue
    Write-ColorOutput "  -OutputFile <file>     - Save results to JSON file" $Colors.Blue
    Write-ColorOutput "  -TimeoutMinutes <int>  - Timeout for individual tests in minutes (default: 10)" $Colors.Blue
    Write-ColorOutput ""
    Write-ColorOutput "Examples:" $Colors.Yellow
    Write-ColorOutput "  .\run_tests.ps1 -TestType all" $Colors.Blue
    Write-ColorOutput "  .\run_tests.ps1 -TestType quick -VerboseOutput" $Colors.Blue
    Write-ColorOutput "  .\run_tests.ps1 -TestType workflow -OutputFile results.json" $Colors.Blue
    Write-ColorOutput "  .\run_tests.ps1 -TestType retrieval -VerboseOutput" $Colors.Blue
    Write-ColorOutput "  .\run_tests.ps1 -TestType model_interface -VerboseOutput" $Colors.Blue
}

# Main execution
try {
    Write-Header "Kolosal Agent System Test Runner"
    
    Write-ColorOutput "Test Type: $TestType" $Colors.Blue
    Write-ColorOutput "Build First: $BuildFirst" $Colors.Blue
    Write-ColorOutput "Verbose: $VerboseOutput" $Colors.Blue
    Write-ColorOutput "Timeout: $TimeoutMinutes minutes" $Colors.Blue
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
