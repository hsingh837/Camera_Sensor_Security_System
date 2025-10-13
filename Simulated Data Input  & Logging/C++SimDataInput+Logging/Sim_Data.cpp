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
#elif defined(__linux__)
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

static fs::path getExecutableDir() {
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) throw std::runtime_error("GetModuleFileNameW failed");
    return fs::path(buf).parent_path();

#elif defined(__APPLE__)
    uint32_tsize = 0;
    _NSGetExecutablePath(nullptr, &size); //get required size
    std::string tmp(size, '/0');
    if (_NSGetExecutablePath(tmp.data(), &size) != 0)
        throw std::runtime_error("_NSGetExecutablePath failed");
    return fs::weakly_canonical(fs::path(tmp).parent_path());
#elif defined(__linux__)\
    char buf[4096];
    ssize_t len = readlink("proc/self/exe", buf, sizeof(buf)-1);
    if (len == -1) throw std::runtime_error("readlink(/proc/self/exe) failed");
    buf[len] = '/0'
    return fs::path(buf.parent_path());
#else
    //we can try argv[0] at runtime as a fallback. We can either do it from main ie. passing it in or using current_path
    return fs::current_path();

#endif
}