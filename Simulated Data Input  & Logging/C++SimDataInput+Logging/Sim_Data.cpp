//same thing as simulated data.C to build a skeleton of what we are looking to do.
//A goal with this is to ensure no directory issues occur across MacOS, Windows, and Linux systems.

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <iostream>

#if defined (_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__LINUX__)
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

static fs::path getExecutableDir() {
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) throw std::runtime_error("GetModuleFileNameW failed");
    return fs::path(buf).parent_path();

//    #elif defined(__APPLE__)
//uint32_tsize = 0;

#endif
}