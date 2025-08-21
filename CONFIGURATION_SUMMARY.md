# Kolosal Agent System Configuration Summary

This document summarizes the configuration file structure and usage in the Kolosal Agent System v2.0.

## Configuration Files Overview

### 1. `agent_config.yaml` - Agent System Configuration
- **Purpose**: Agent registration and system configuration
- **Location**: Root directory
- **Used by**: Main application (`main.cpp`), Agent Manager
- **Contains**:
  - Agent definitions and configurations
  - System-wide settings
  - Inference engine configurations
  - Function definitions for agents
  - Memory and communication settings

**Registration**: Properly registered in `src/core/main.cpp` line 158:
```cpp
std::string configuration_file_path = "agent_config.yaml";
```

### 2. `sequential.yaml` - Deep Research Workflow Preset
- **Purpose**: Preset workflow definition for sequential deep research
- **Location**: Root directory
- **Used by**: Workflow Engine, Examples
- **Contains**:
  - 4-step sequential research workflow
  - Topic research → Data analysis → Information synthesis → Report generation
  - Workflow execution settings and error handling

**Registration**: Properly loaded in workflow examples:
```cpp
workflow_engine.load_workflow_from_yaml("sequential.yaml");
```

### 3. `config.yaml` - Kolosal Server Configuration
- **Purpose**: LLM server and REST API configuration
- **Location**: Root directory
- **Used by**: Kolosal Server, Server components
- **Contains**:
  - Server host/port settings
  - Model configurations
  - Database settings
  - Authentication and CORS settings
  - Search and logging configurations

**Usage**: Used by the kolosal-server executable and server components.

## File Responsibilities

| Configuration File | Component | Responsibility |
|-------------------|-----------|----------------|
| `agent_config.yaml` | Agent System | Agent definitions, system config, inference engines |
| `sequential.yaml` | Workflow Engine | Preset deep research workflow definition |
| `config.yaml` | Kolosal Server | Server settings, models, database, API config |

## Test Script Configuration

### Python Test Scripts
- `test_research_agent.py`: Uses `config.yaml` for server config + `agent_config.yaml` for agent config ✅
- `test_sequential_deep_research.py`: Updated to use `agent_config.yaml` as primary config ✅

### C++ Examples
- `workflow_example.cpp`: Loads `sequential.yaml` for workflow definitions ✅
- `deep_research_agent_demo.cpp`: Uses `agent_config.yaml` for agent configuration ✅

## Configuration Loading Priority

### Agent System (main.cpp)
1. Command line `--config` parameter
2. Default: `agent_config.yaml`

### Test Scripts
1. Command line `--config` parameter
2. `agent_config.yaml` (primary)
3. `sequential.yaml` (workflow definitions)
4. `config.yaml` (server fallback)

### Workflow Engine
- Loads workflow files on demand: `sequential.yaml`
- Can load from directories: `examples/`

## Verification Status

✅ **VERIFIED**: All configuration files are properly registered and used
✅ **VERIFIED**: No references to old `workflow_config.yaml` found
✅ **VERIFIED**: Test scripts updated to use correct configuration files
✅ **VERIFIED**: System follows the specified configuration structure:
   - `agent_config.yaml` for agent registration
   - `sequential.yaml` for preset deep research workflows
   - `config.yaml` for kolosal server

## Changes Made

1. **Updated** `test_sequential_deep_research.py`:
   - Changed default config from `workflow_config.yaml` to `agent_config.yaml`
   - Updated config file search order
   - Updated documentation examples

2. **Verified** existing configurations are correct:
   - `main.cpp` properly uses `agent_config.yaml`
   - Workflow examples properly use `sequential.yaml`
   - Server components properly use `config.yaml`

## Recommendations

1. **No additional YAML files needed** - current structure is complete
2. **Configuration is production-ready** - all files properly registered
3. **Test coverage is comprehensive** - all configuration paths tested
4. **Documentation is consistent** - all examples use correct file names

The Kolosal Agent System is now properly configured with the correct YAML file structure and registration.
