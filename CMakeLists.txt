cmake_minimum_required(VERSION 3.14)
project(NovaVCS VERSION 0.1.0 LANGUAGES CXX)

# Require C++17 for std::filesystem and modern features (Ubuntu compatible)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable strong warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)

# --- ADD THIS LINE ---
enable_testing() 

add_subdirectory(tests)
