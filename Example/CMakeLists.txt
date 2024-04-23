cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(DirectX11FSR1Example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
    main.cpp
)

add_executable(DirectX11FSR1Example ${SOURCE_FILES}) 
target_link_libraries(DirectX11FSR1Example PRIVATE SimpleFSR1)