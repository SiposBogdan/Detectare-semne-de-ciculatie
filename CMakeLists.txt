cmake_minimum_required(VERSION 3.14)
project(sign_detection)

set(CMAKE_CXX_STANDARD 17)

# --- Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(sign_detection
        main.cpp
)

target_link_libraries(sign_detection ${OpenCV_LIBS})
