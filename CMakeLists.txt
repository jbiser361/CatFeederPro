cmake_minimum_required(VERSION 3.16)
project(CatFeederPro VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/embedded-libs)

add_executable(catfeeder
    src/main.cpp
)

target_link_libraries(catfeeder
    PRIVATE
    display
    rfid
    distance
)
