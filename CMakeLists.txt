cmake_minimum_required(VERSION 3.10.0)
project(testfolder VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Set Catch2 (or another testing framework) include path if needed
# include_directories(${CMAKE_SOURCE_DIR}/path/to/catch2)

# Define source files
set(SRC_FILES scheduler.cpp)

# Main executable (interactive CLI scheduler)
add_executable(scheduler main.cpp ${SRC_FILES})

# Test executable (unit tests)
add_executable(tests tests.cpp ${SRC_FILES})

# Enable testing
include(CTest)
enable_testing()