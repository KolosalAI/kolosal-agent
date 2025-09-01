# Test Runners for Kolosal Agent System

This directory contains comprehensive test runners for the Kolosal Agent System.

## Available Test Runners

### 1. PowerShell Script (Windows)
- **File**: `run_tests.ps1`
- **Platform**: Windows PowerShell/PowerShell Core
- **Usage**: `.\run_tests.ps1 [options]`

### 2. Shell Script (Linux/macOS/WSL)
- **File**: `run_tests.sh`  
- **Platform**: Linux, macOS, WSL
- **Usage**: `./run_tests.sh [options]`

## Test Types

Both scripts support the following test types:

- **`all`** (default): Run all available tests
- **`cpp`**: C++ unit tests only (GoogleTest)
- **`integration`**: Python integration tests
- **`comprehensive`**: Comprehensive integration test suite
- **`health`**: System health checks
- **`server`**: Kolosal Server specific tests

## Usage Examples

### PowerShell (Windows)

```powershell
# Run all tests
.\run_tests.ps1

# Run only C++ unit tests
.\run_tests.ps1 -TestType cpp

# Run integration tests with verbose output
.\run_tests.ps1 -TestType integration -VerboseOutput

# Run all tests with Release configuration, skip build
.\run_tests.ps1 -ConfigType Release -SkipBuild

# Run comprehensive tests with custom server URL
.\run_tests.ps1 -TestType comprehensive -ServerUrl "http://localhost:9090"
```

### Shell Script (Linux/macOS/WSL)

```bash
# Run all tests
./run_tests.sh

# Run only C++ unit tests
./run_tests.sh -t cpp

# Run integration tests with verbose output
./run_tests.sh -t integration -v

# Run all tests with Release configuration, skip build
./run_tests.sh -c Release -s

# Run comprehensive tests with custom server URL
./run_tests.sh -t comprehensive -u "http://localhost:9090"
```

## Command Line Options

### PowerShell Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `-TestType` | String | `all` | Type of tests to run |
| `-ConfigType` | String | `Debug` | Build configuration |
| `-ServerUrl` | String | `http://localhost:8080` | Server URL for integration tests |
| `-SkipBuild` | Switch | `false` | Skip building tests |
| `-VerboseOutput` | Switch | `false` | Enable verbose output |
| `-BuildTests` | Bool | `true` | Build C++ tests before running |

### Shell Script Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `-t, --test-type` | String | `all` | Type of tests to run |
| `-c, --config` | String | `Debug` | Build configuration |
| `-u, --url` | String | `http://localhost:8080` | Server URL for integration tests |
| `-s, --skip-build` | Flag | `false` | Skip building tests |
| `-v, --verbose` | Flag | `false` | Enable verbose output |
| `-h, --help` | Flag | - | Show help message |

## Prerequisites

Both scripts will automatically check for and report missing prerequisites:

### Required Tools
- **CMake**: For building C++ tests
- **Python**: For integration tests (version 3.6+)
- **C++ Compiler**: Visual Studio 2017+ on Windows, GCC/Clang on Linux/macOS

### Required Python Packages
- `requests`: For HTTP communication with the agent system
- Additional packages will be installed automatically as needed

## Test Structure

### C++ Unit Tests
- **Location**: `tests/*.cpp`
- **Framework**: GoogleTest/GoogleMock
- **Executable**: `build/kolosal_agent_tests.exe` (Windows) or `build/kolosal_agent_tests` (Linux/macOS)
- **Configuration**: Controlled by `BUILD_TESTS` CMake option

### Python Integration Tests
- **Basic Integration**: `test_integration.py` - Basic system functionality
- **Comprehensive**: `scripts/comprehensive_test.py` - Full test suite with concurrent testing
- **Health Check**: `scripts/health_check.py` - System health validation

### Server Tests
- **Location**: Built Kolosal Server test executables
- **Type**: Server-specific functionality tests

## Output and Reporting

### Console Output
Both scripts provide:
- Color-coded test results (✅ Pass, ❌ Fail, ⚠️ Skip)
- Progress indicators
- Detailed summary with success rates and timing

### Test Reports
- **XML Reports**: GoogleTest generates XML reports for C++ tests
- **JSON Reports**: Comprehensive tests can generate detailed JSON reports
- **Log Files**: System logs are preserved for debugging

## Exit Codes

| Code | Meaning |
|------|---------|
| `0` | All tests passed successfully |
| `1` | Some tests failed or partial success |
| `2` | Critical failure or system error |
| `130` | Interrupted by user (Ctrl+C) |

## Integration with CI/CD

Both scripts are designed for CI/CD integration:

```yaml
# Example GitHub Actions usage
- name: Run Tests
  run: |
    cd tests
    ./run_tests.sh -t all -v
```

```powershell
# Example PowerShell CI usage
cd tests
.\run_tests.ps1 -TestType all -VerboseOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
```

## Troubleshooting

### Common Issues

1. **CMake not found**: Install CMake and add to PATH
2. **Python not found**: Install Python 3.6+ and add to PATH  
3. **Build failures**: Ensure all dependencies are available
4. **Test executable not found**: Run with build enabled first
5. **Server connection issues**: Check if agent system is running

### Debug Mode

Use `-VerboseOutput` (PowerShell) or `-v` (Shell) for detailed output:
- CMake configuration details
- Build output
- Full test execution logs
- Detailed error messages

### Manual Execution

If scripts fail, tests can be run manually:

```bash
# Build tests
cd build
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target kolosal_agent_tests

# Run C++ tests
./kolosal_agent_tests

# Run Python tests
python ../test_integration.py
python ../scripts/comprehensive_test.py
```

## Support

For issues related to test execution:
1. Check prerequisites are installed
2. Run with verbose output for debugging
3. Check project documentation in `docs/`
4. Verify system configuration matches requirements
