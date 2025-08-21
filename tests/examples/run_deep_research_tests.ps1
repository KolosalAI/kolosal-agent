# Deep Research Agent Test Runner (PowerShell)
# Comprehensive test execution script for the Deep Research Agent on Windows

param(
    [string]$TestType = "unit",
    [string]$BuildDir = "build",
    [switch]$Verbose,
    [switch]$Coverage,
    [string]$ServerUrl = "http://localhost:8080",
    [switch]$Help
)

# Function to print colored output
function Write-ColoredOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

# Function to print usage
function Show-Usage {
    Write-Host "Usage: .\run_deep_research_tests.ps1 [OPTIONS]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -TestType TYPE     Test type: unit, integration, performance, all (default: unit)"
    Write-Host "  -BuildDir DIR      Build directory (default: build)"
    Write-Host "  -Verbose           Enable verbose output"
    Write-Host "  -Coverage          Enable test coverage reporting"
    Write-Host "  -ServerUrl URL     Server URL for integration tests (default: http://localhost:8080)"
    Write-Host "  -Help              Show this help message"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\run_deep_research_tests.ps1 -TestType unit"
    Write-Host "  .\run_deep_research_tests.ps1 -TestType integration -Verbose"
    Write-Host "  .\run_deep_research_tests.ps1 -TestType all -Coverage"
    Write-Host "  .\run_deep_research_tests.ps1 -TestType performance -ServerUrl http://localhost:9090"
}

# Show help if requested
if ($Help) {
    Show-Usage
    exit 0
}

# Validate test type
if ($TestType -notin @("unit", "integration", "performance", "all")) {
    Write-ColoredOutput "Error: Invalid test type '$TestType'" "Red"
    Show-Usage
    exit 1
}

# Check if build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-ColoredOutput "Error: Build directory '$BuildDir' does not exist" "Red"
    exit 1
}

# Change to build directory
Set-Location $BuildDir

Write-ColoredOutput "=== Deep Research Agent Test Runner ===" "Blue"
Write-ColoredOutput "Test Type: $TestType" "Blue"
Write-ColoredOutput "Build Directory: $BuildDir" "Blue"
Write-ColoredOutput "Verbose: $Verbose" "Blue"
Write-ColoredOutput "Coverage: $Coverage" "Blue"
Write-ColoredOutput "Server URL: $ServerUrl" "Blue"
Write-ColoredOutput "========================================" "Blue"

# Set environment variables for integration tests
if ($TestType -eq "integration" -or $TestType -eq "all") {
    $env:KOLOSAL_INTEGRATION_TESTS = "1"
    $env:KOLOSAL_SERVER_URL = $ServerUrl
    Write-ColoredOutput "Integration tests enabled with server: $ServerUrl" "Yellow"
}

# Set environment variables for performance tests
if ($TestType -eq "performance" -or $TestType -eq "all") {
    $env:KOLOSAL_PERFORMANCE_TESTS = "1"
    Write-ColoredOutput "Performance tests enabled" "Yellow"
}

# Function to run unit tests
function Invoke-UnitTests {
    Write-ColoredOutput "`n=== Running Unit Tests ===" "Green"
    
    $testExe = $null
    if (Test-Path ".\Debug\kolosal_agent_unit_tests.exe") {
        $testExe = ".\Debug\kolosal_agent_unit_tests.exe"
    } elseif (Test-Path ".\Release\kolosal_agent_unit_tests.exe") {
        $testExe = ".\Release\kolosal_agent_unit_tests.exe"
    } elseif (Test-Path ".\kolosal_agent_unit_tests.exe") {
        $testExe = ".\kolosal_agent_unit_tests.exe"
    }
    
    if (-not $testExe) {
        Write-ColoredOutput "Error: Unit test executable not found" "Red"
        return $false
    }
    
    $testArgs = @()
    if ($Verbose) {
        $testArgs += "--gtest_output=xml:unit_test_results.xml"
    }
    
    Write-ColoredOutput "Command: $testExe $($testArgs -join ' ')" "Blue"
    
    try {
        if ($testArgs.Count -gt 0) {
            & $testExe $testArgs
        } else {
            & $testExe
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-ColoredOutput "✓ Unit tests passed" "Green"
            return $true
        } else {
            Write-ColoredOutput "✗ Unit tests failed" "Red"
            return $false
        }
    } catch {
        Write-ColoredOutput "✗ Unit tests failed with exception: $_" "Red"
        return $false
    }
}

# Function to run integration tests
function Invoke-IntegrationTests {
    Write-ColoredOutput "`n=== Running Integration Tests ===" "Green"
    
    $testExe = $null
    if (Test-Path ".\Debug\kolosal_agent_integration_tests.exe") {
        $testExe = ".\Debug\kolosal_agent_integration_tests.exe"
    } elseif (Test-Path ".\Release\kolosal_agent_integration_tests.exe") {
        $testExe = ".\Release\kolosal_agent_integration_tests.exe"
    } elseif (Test-Path ".\kolosal_agent_integration_tests.exe") {
        $testExe = ".\kolosal_agent_integration_tests.exe"
    }
    
    if (-not $testExe) {
        Write-ColoredOutput "Warning: Integration test executable not found, skipping" "Yellow"
        return $true
    }
    
    $testArgs = @()
    if ($Verbose) {
        $testArgs += "--gtest_output=xml:integration_test_results.xml"
    }
    
    Write-ColoredOutput "Command: $testExe $($testArgs -join ' ')" "Blue"
    
    # Check server connectivity
    Write-ColoredOutput "Checking server connectivity at $ServerUrl..." "Yellow"
    try {
        $response = Invoke-WebRequest -Uri "$ServerUrl/health" -TimeoutSec 5 -ErrorAction SilentlyContinue
        if (-not $response) {
            Write-ColoredOutput "Warning: Server not reachable at $ServerUrl" "Yellow"
            Write-ColoredOutput "Integration tests may skip server-dependent tests" "Yellow"
        }
    } catch {
        Write-ColoredOutput "Warning: Server not reachable at $ServerUrl" "Yellow"
        Write-ColoredOutput "Integration tests may skip server-dependent tests" "Yellow"
    }
    
    try {
        if ($testArgs.Count -gt 0) {
            & $testExe $testArgs
        } else {
            & $testExe
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-ColoredOutput "✓ Integration tests passed" "Green"
            return $true
        } else {
            Write-ColoredOutput "✗ Integration tests failed" "Red"
            return $false
        }
    } catch {
        Write-ColoredOutput "✗ Integration tests failed with exception: $_" "Red"
        return $false
    }
}

# Function to run performance tests
function Invoke-PerformanceTests {
    Write-ColoredOutput "`n=== Running Performance Tests ===" "Green"
    
    $testExe = $null
    if (Test-Path ".\Debug\kolosal_agent_performance_tests.exe") {
        $testExe = ".\Debug\kolosal_agent_performance_tests.exe"
    } elseif (Test-Path ".\Release\kolosal_agent_performance_tests.exe") {
        $testExe = ".\Release\kolosal_agent_performance_tests.exe"
    } elseif (Test-Path ".\kolosal_agent_performance_tests.exe") {
        $testExe = ".\kolosal_agent_performance_tests.exe"
    }
    
    if (-not $testExe) {
        Write-ColoredOutput "Warning: Performance test executable not found, skipping" "Yellow"
        return $true
    }
    
    $testArgs = @()
    if ($Verbose) {
        $testArgs += "--gtest_output=xml:performance_test_results.xml"
    }
    
    Write-ColoredOutput "Command: $testExe $($testArgs -join ' ')" "Blue"
    Write-ColoredOutput "Note: Performance tests may take several minutes to complete" "Yellow"
    
    try {
        if ($testArgs.Count -gt 0) {
            & $testExe $testArgs
        } else {
            & $testExe
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-ColoredOutput "✓ Performance tests passed" "Green"
            return $true
        } else {
            Write-ColoredOutput "✗ Performance tests failed" "Red"
            return $false
        }
    } catch {
        Write-ColoredOutput "✗ Performance tests failed with exception: $_" "Red"
        return $false
    }
}

# Function to generate coverage report
function New-CoverageReport {
    if ($Coverage) {
        Write-ColoredOutput "`n=== Generating Coverage Report ===" "Green"
        
        # Check if OpenCppCoverage is available (Windows coverage tool)
        $opencppcoverage = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
        if ($opencppcoverage) {
            Write-ColoredOutput "Generating coverage report with OpenCppCoverage..." "Blue"
            # This would require re-running tests with coverage enabled
            Write-ColoredOutput "Coverage reporting with OpenCppCoverage requires test re-execution" "Yellow"
        } else {
            Write-ColoredOutput "Warning: OpenCppCoverage not found, skipping coverage report" "Yellow"
            Write-ColoredOutput "Install OpenCppCoverage for Windows coverage reporting" "Yellow"
        }
    }
}

# Main execution
$exitCode = 0

switch ($TestType) {
    "unit" {
        if (-not (Invoke-UnitTests)) { $exitCode = 1 }
    }
    "integration" {
        if (-not (Invoke-IntegrationTests)) { $exitCode = 1 }
    }
    "performance" {
        if (-not (Invoke-PerformanceTests)) { $exitCode = 1 }
    }
    "all" {
        if (-not (Invoke-UnitTests)) { $exitCode = 1 }
        if (-not (Invoke-IntegrationTests)) { $exitCode = 1 }
        if (-not (Invoke-PerformanceTests)) { $exitCode = 1 }
    }
}

New-CoverageReport

Write-ColoredOutput "`n========================================" "Blue"
if ($exitCode -eq 0) {
    Write-ColoredOutput "✓ All requested tests completed successfully" "Green"
} else {
    Write-ColoredOutput "✗ Some tests failed" "Red"
}
Write-ColoredOutput "========================================" "Blue"

exit $exitCode
