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
#elif defined(__linux__)
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

int main() {
    try {
        fs::path exe_dir = getExecutableDir();
        fs::path data_dir = exe_dir / "C:/Users/harsh/Harshal/Coding/Projects/GitHub/sensor_data_logger/Simulated Data Input  & Logging/C++SimDataInput+Logging/data";
        fs::create_directories(data_dir);
        fs::path csv_path = data_dir / "sensor_log_cpp.csv";
    
        //Open CSV to write simulated data into
        FILE* f = std::fopen(csv_path.string().c_str(), "w");
        if (!f) { std::perror("Failed to open file"); return 1; }

        std::srand(static_cast<unsigned>(std::time(nullptr)));
        std::fprintf(f, "timestamp,, temperature_C,, pressure_kPa,, color_(R-G-B)),,, brightness (Lumens)\n");


        for (int i = 0; i < 99; ++i) { //This is where we start to see matching between our C program and this one, the loop to create our simulated data
            int temp = 20 + std::rand() % 10; //range of 20-29 Celsius
            int pressure = 100 + std::rand() % 11; //range of 100 - 110 kPa

            int r = std::rand() % 256;
            int g = std::rand() % 256; //All 3 create colors on the RGB spectrum randomly to simulate change in data for color sensor
            int b = std::rand() % 256;

            int brightness = std::rand() % 21; //random brightness levels in any unit from 0 - 20

            std::time_t now = std::time(nullptr);

            std::fprintf(
                f, "%lld,,%d,,%d,,(%d, %d, %d), %d\n",
                static_cast<long long>(now), temp, pressure, r, g, b, brightness
        
            );
        
        }

        std::fclose(f);
        std::cout <<"Simulated Sensor Data successfully logged into file:/n"
        << csv_path << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " <<e.what() << std::endl;
         return 2;
        
    }
}



