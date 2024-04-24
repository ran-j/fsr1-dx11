# Simple FSR 1 for Directx 11

This is a simple implementation of FSR 1.0 for Directx 11. The implementation is based on the Directx 11 API and uses the Compute Shader to perform the upscaling. The implementation is very simple and is intended to be used as a starting point for developers who want to integrate FSR 1.0 into their Directx 11 applications.

# How to use

The implementation is very simple and can be integrated into any Directx 11 application. The implementation consists of one main class: `SimpleFSR1`. The class is responsible for create and calling the compute shader to perform the upscaling.

To use the implementation, you need to create an instance of the `SimpleFSR1` class and call the `Upscale` method to perform the upscaling.

# Installation

To use the implementation, you will need cmake and a C++ compiler that supports C++11.

```cmake
cmake_minimum_required(VERSION 3.0)
project(my-dx-app)

include(FetchContent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_Declare(
  FSR1DX11
  GIT_REPOSITORY https://github.com/ran-j/fsr1-dx11.git
  GIT_TAG main
  GIT_PROGRESS TRUE
)
d
FetchContent_MakeAvailable(FSR1DX11)

add_executable(my-dx-app main.cpp)
target_link_libraries(my-dx-app PRIVATE fsr1-dx11)
```
