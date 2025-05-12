cmake_minimum_required(VERSION 3.10.0)
project(testfolder VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SFML root directory (adjust as needed)
set(SFML_ROOT "C:/SFML-2.5.1")
include_directories("${SFML_ROOT}/include")
link_directories("${SFML_ROOT}/lib")

# Define source files
set(SRC_FILES scheduler.cpp)

# Detect build type (default to Release if not specified)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set SFML libraries depending on build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SFML_LIBS
        sfml-graphics-d
        sfml-window-d
        sfml-system-d
    )
else()
    set(SFML_LIBS
        sfml-graphics
        sfml-window
        sfml-system
    )
endif()

# Main executable
add_executable(scheduler main.cpp ${SRC_FILES})
target_link_libraries(scheduler ${SFML_LIBS})

# Test executable
add_executable(tests tests.cpp ${SRC_FILES})
target_link_libraries(tests ${SFML_LIBS})

include(CTest)
enable_testing()