/**
 * @file export.hpp
 * @brief Core functionality for export
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

// Export/import macros for Windows DLL support
#ifdef _WIN32
    #ifdef KOLOSAL_AGENT_STATIC
        // Static library - no exports/imports needed
        #define KOLOSAL_AGENT_API
    #elif defined(KOLOSAL_AGENT_BUILD)
        // Building the library - export symbols
        #define KOLOSAL_AGENT_API __declspec(dllexport)
    #elif defined(KOLOSAL_AGENT_SHARED)
        // Using shared library - import symbols
        #define KOLOSAL_AGENT_API __declspec(dllimport)
    #else
        // Default to static for executables linking against the library
        #define KOLOSAL_AGENT_API
    #endif
#else
    // For Unix-based systems
    #ifdef KOLOSAL_AGENT_BUILD
        #define KOLOSAL_AGENT_API __attribute__((visibility("default")))
    #else
        #define KOLOSAL_AGENT_API
    #endif
#endif

// Use the server's export macro for compatibility
#ifdef KOLOSAL_SERVER_BUILD
    // If building as part of server, use server API
    #ifndef KOLOSAL_SERVER_API
        #define KOLOSAL_SERVER_API KOLOSAL_AGENT_API
    #endif
#else
    // If building standalone agent, define server API as import
    #ifndef KOLOSAL_SERVER_API
        #ifdef _WIN32
            #ifdef KOLOSAL_AGENT_STATIC
                #define KOLOSAL_SERVER_API
            #else
                #define KOLOSAL_SERVER_API __declspec(dllimport)
            #endif
        #else
            #define KOLOSAL_SERVER_API
        #endif
    #endif
#endif
