# Functions API Reference

Generated on 2025-08-08 14:06:25

## checkKernel_Modules

**File:** `naming_backup\kolosal-server\src\gpu_detection.cpp`  
**Line:** 265  
**Return Type:** `bool`  

```cpp
bool checkKernel_Modules()
```

**Description:** Method 1: Check loaded kernel modules /** * @brief Perform checkkernel modules operation * @return bool Description of return value */

---

## checkVulkan_Support

**File:** `naming_backup\kolosal-server\src\gpu_detection.cpp`  
**Line:** 366  
**Return Type:** `bool`  

```cpp
bool checkVulkan_Support()
```

**Description:** Method 4: Check for Vulkan support /** * @brief Perform checkvulkan support operation * @return bool Description of return value */

---

## check_DRMDevices

**File:** `naming_backup\kolosal-server\src\gpu_detection.cpp`  
**Line:** 292  
**Return Type:** `bool`  

```cpp
bool check_DRMDevices()
```

**Description:** Method 2: Check DRM devices /** * @brief Perform check drmdevices operation * @return bool Description of return value */

---

## check_Lspci

**File:** `naming_backup\kolosal-server\src\gpu_detection.cpp`  
**Line:** 336  
**Return Type:** `bool`  

```cpp
bool check_Lspci()
```

**Description:** Method 3: Use lspci command /** * @brief Perform check lspci operation * @return bool Description of return value */

---

## configureUPnPPort_Forwarding

**File:** `naming_backup\kolosal-server\src\main.cpp`  
**Line:** 199  
**Return Type:** `bool`  

```cpp
bool configureUPnPPort_Forwarding(const std::string &port)
```

**Description:** Function to attempt UPnP port forwarding /** * @brief Perform configureupnpport forwarding operation * @return bool Description of return value */

---

## contains_GPUVendor

**File:** `naming_backup\kolosal-server\src\gpu_detection.cpp`  
**Line:** 243  
**Return Type:** `bool`  

```cpp
bool contains_GPUVendor(const std::string& text)
```

**Description:** Check if string contains any of the GPU vendor names (case insensitive) /** * @brief Perform contains gpuvendor operation * @return bool Description of return value */

---

## display_application_banner

**File:** `src\main.cpp`  
**Line:** 272  
**Return Type:** `void`  

```cpp
void display_application_banner()
```

**Description:** /** * @brief Display the application banner with version and feature information */

---

## display_application_usage_information

**File:** `src\main.cpp`  
**Line:** 202  
**Return Type:** `void`  

```cpp
void display_application_usage_information(const char* program_executable_name)
```

**Description:** /** * @brief Display comprehensive usage information and help text * @param program_executable_name The name of the program executable */

---

## execute_system_demonstration

**File:** `src\main.cpp`  
**Line:** 619  
**Return Type:** `void`  

```cpp
void execute_system_demonstration(const UnifiedKolosalServer& unified_server_ref)
```

**Description:** /** * @brief Execute system demonstration showcasing key features and capabilities * @param unified_server_ref Reference to the unified server instance */

---

## file_Exists

**File:** `src\server_client_interface.cpp`  
**Line:** 103  
**Return Type:** `bool`  

```cpp
bool file_Exists(const std::string& path)
```

**Description:** Helper function to check if file exists /** * @brief Perform file exists operation * @return bool Description of return value */

---

## findServer_Process

**File:** `src\server_client_interface.cpp`  
**Line:** 112  
**Return Type:** `ProcessId`  

```cpp
ProcessId findServer_Process()
```

**Description:** Helper function to find server process ID /** * @brief Perform findserver process operation * @return ProcessId Description of return value */

---

## get__directory_from_path

**File:** `src\kolosal_launcher.c`  
**Line:** 36  
**Return Type:** `void`  

```cpp
void get__directory_from_path(const char *path, char *dir, size_t dir_size)
```

**Description:** Cross-platform function to get directory from path /** * @brief Get   directory from path * @return void Description of return value */

---

## hasVulkanCapable_GPU

**File:** `naming_backup\kolosal-server\src\gpu_detection.cpp`  
**Line:** 390  
**Return Type:** `bool`  

```cpp
bool hasVulkanCapable_GPU()
```

**Description:** /** * @brief Check if has vulkancapable gpu * @return bool Description of return value */

---

## initialize_ialize_default_configuration_if_missing

**File:** `src\main.cpp`  
**Line:** 287  
**Return Type:** `void`  

```cpp
void initialize_ialize_default_configuration_if_missing(const std::string& configuration_file_path)
```

**Description:** /** * @brief Create default configuration file if it doesn't exist * @param configuration_file_path Path to the configuration file */

---

## initialize_ialize_system_health_monitoring

**File:** `src\main.cpp`  
**Line:** 595  
**Return Type:** `void`  

```cpp
void initialize_ialize_system_health_monitoring(const UnifiedKolosalServer& unified_server_ref)
```

**Description:** /** * @brief Setup comprehensive health monitoring for the unified server * @param unified_server_ref Reference to the unified server instance */

---

## main

**File:** `src\kolosal_launcher.c`  
**Line:** 65  
**Return Type:** `int`  

```cpp
int main(int argc, char *argv[])
```

**Description:** /** * @brief Perform main operation * @return int Description of return value */

---

## parse_command_line_arguments

**File:** `src\main.cpp`  
**Line:** 130  
**Return Type:** `ApplicationConfiguration`  

```cpp
ApplicationConfiguration parse_command_line_arguments(int argc, char* argv[])
```

**Description:** * @param argc Argument count * @param argv Argument vector * @return ApplicationConfiguration with parsed values * @throws std::runtime_error for invalid arguments */ /** * @brief Perform parse command line arguments operation * @return ApplicationConfiguration Description of return value */

---

## print_usage

**File:** `naming_backup\kolosal-server\src\main.cpp`  
**Line:** 244  
**Return Type:** `void`  

```cpp
void print_usage(const char *program_name)
```

**Description:** /** * @brief Perform print usage operation * @return void Description of return value */

---

## print_version

**File:** `naming_backup\kolosal-server\src\main.cpp`  
**Line:** 254  
**Return Type:** `void`  

```cpp
void print_version()
```

**Description:** /** * @brief Perform print version operation * @return void Description of return value */

---

## signal_handler

**File:** `naming_backup\kolosal-server\src\main.cpp`  
**Line:** 234  
**Return Type:** `void`  

```cpp
void signal_handler(int signal)
```

**Description:** Signal handler for graceful shutdown /** * @brief Perform signal handler operation * @return void Description of return value */

---

## solution

**File:** `src\builtin_function_registry.cpp`  
**Line:** 1120  
**Return Type:** `function`  

```cpp
function solution()
```

---

## system_signal_handler

**File:** `src\main.cpp`  
**Line:** 75  
**Return Type:** `void`  

```cpp
void system_signal_handler(int signal_number)
```

**Description:** /** * @brief Enhanced signal handler for graceful system shutdown * @param signal_number The received signal number */

---

## terminate_Process

**File:** `src\server_client_interface.cpp`  
**Line:** 191  
**Return Type:** `bool`  

```cpp
bool terminate_Process(ProcessId pid)
```

**Description:** Helper function to terminate process /** * @brief Perform terminate process operation * @return bool Description of return value */

---

