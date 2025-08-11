/**
 * @file simple_test_main.cpp
 * @brief Simple test program to verify Windows dialog fixes
 */

#include <iostream>
#include <csignal>

#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>
#endif

void disable_windows_error_dialogs() {
#ifdef _WIN32
    // Disable the Windows Error Reporting dialog
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    
    // Disable debug assertion dialogs in Debug builds
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    
    // Disable abort() dialog
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    
    // Set unhandled exception filter
    SetUnhandledExceptionFilter([](PEXCEPTION_POINTERS pExceptionInfo) -> LONG {
        std::cerr << "Fatal: Unhandled exception - terminating silently" << std::endl;
        return EXCEPTION_EXECUTE_HANDLER;
    });
    
    // Redirect abort() to our custom handler
    signal(SIGABRT, [](int) {
        std::cerr << "Fatal: Abort signal - terminating silently" << std::endl;
        std::_Exit(3);
    });
#endif
}

int main() {
    std::cout << "Testing Windows error dialog fixes..." << std::endl;
    
    // Install the fix first
    disable_windows_error_dialogs();
    
    std::cout << "Error dialogs have been disabled. Testing..." << std::endl;
    
    // Test 1: abort() - should not show dialog
    std::cout << "Test 1: Calling abort() - should exit silently without dialog" << std::endl;
    std::abort();  // This should exit without showing a dialog
    
    return 0;
}
