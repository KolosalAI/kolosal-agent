# Installation Guide

Step-by-step guide to install and set up the Kolosal Agent System.

## Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Git with submodule support
- 4GB RAM minimum (8GB+ recommended)

## Build Instructions

### Quick Start

```bash
git clone --recursive https://github.com/Evintkoo/kolosal-agent.git
cd kolosal-agent
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

