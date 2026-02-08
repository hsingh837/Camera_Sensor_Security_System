cmake_minimum_required(VERSION 3.10)
project(CppMotionSensor)

# Enabling C++17 (needed for filesystem)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json (editor / LSP support)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# -------------------------------------------------
# Program 1: Single-camera baseline
# -------------------------------------------------
add_executable(motion_single
    src/main.cpp
)
target_link_libraries(motion_single
    ${OpenCV_LIBS}
)

# -------------------------------------------------
# Program 2: Dual-camera, non-threaded
# -------------------------------------------------
add_executable(motion_dual
    src/main_2Cams.cpp
)
target_link_libraries(motion_dual
    ${OpenCV_LIBS}
)

# # -------------------------------------------------
# # Program 3: Dual-camera, threaded (future)
# # -------------------------------------------------
add_executable(motion_dual_threaded
    src/main_2Cams_Threaded.cpp
)
target_link_libraries(motion_dual_threaded
    ${OpenCV_LIBS}
)
