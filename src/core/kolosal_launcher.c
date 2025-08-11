/**
 * @file kolosal_launcher.c
 * @brief Core functionality for kolosal launcher
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <process.h>
    #define PATH_SEPARATOR "\\"
    #define ACCESS_OK 0
    #define file_exists(path) (_access((path), ACCESS_OK) == 0)
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <libgen.h>
    #define PATH_SEPARATOR "/"
    #define file_exists(path) (access((path), F_OK) == 0)
#endif

// Cross-platform function to get directory from path
/**
 * @brief Extract the directory component from a file path into `dir`.
 */
void get_directory_from_path(const char *path, char *dir, size_t dir_size) {
#ifdef _WIN32
    char drive[_MAX_DRIVE] = {0};
    char directory[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    _splitpath_s(path,
                 drive, (rsize_t)sizeof(drive),
                 directory, (rsize_t)sizeof(directory),
                 fname, (rsize_t)sizeof(fname),
                 ext, (rsize_t)sizeof(ext));

    // Combine drive and directory
    snprintf(dir, dir_size, "%s%s", drive, directory);

    // Remove trailing path separator if present
    size_t len = strlen(dir);
    if (len > 0 && (dir[len - 1] == '\\' || dir[len - 1] == '/')) {
        dir[len - 1] = '\0';
    }
#else
    char *path_copy = strdup(path);
    char *directory = dirname(path_copy);
    strncpy(dir, directory, dir_size - 1);
    dir[dir_size - 1] = '\0';
    free(path_copy);
#endif
}

/**
 * @brief Perform main operation
 * @return int Description of return value
 */
int main(int argc, char *argv[]) {
    // Get the directory of the executable
    char exe_dir[1024];
    get_directory_from_path(argv[0], exe_dir, sizeof(exe_dir));
    
    // Build path to the main agent executable
    char agent_path[1024];
    
#ifdef _WIN32
    snprintf(agent_path, sizeof(agent_path), "%s%s%s", exe_dir, PATH_SEPARATOR, "kolosal-agent-unified.exe");
#else
    snprintf(agent_path, sizeof(agent_path), "%s%s%s", exe_dir, PATH_SEPARATOR, "kolosal-agent-unified");
#endif
    
    // Check if the agent executable exists
    if (!file_exists(agent_path)) {
        fprintf(stderr, "Error: kolosal-agent executable not found at %s\n", agent_path);
        return 1;
    }
    
    // Execute the agent with the same arguments (excluding argv[0])
    char **new_argv = malloc((argc + 1) * sizeof(char*));
    if (!new_argv) {
        fprintf(stderr, "Error: Failed to allocate memory for arguments\n");
        return 1;
    }
    
    new_argv[0] = agent_path;
    for (int i = 1; i < argc; i++) {
        new_argv[i] = argv[i];
    }
    new_argv[argc] = NULL;
    
#ifdef _WIN32
    // Use _execv on Windows
    const intptr_t result = _execv(agent_path, new_argv);
    free(new_argv);
    if (result == -1) {
        perror("Failed to execute kolosal-agent");
        return 1;
    }
#else
    // Use execv on Unix systems
    if (execv(agent_path, new_argv) == -1) {
        free(new_argv);
        perror("Failed to execute kolosal-agent");
        return 1;
    }
#endif
    
    return 0;
}