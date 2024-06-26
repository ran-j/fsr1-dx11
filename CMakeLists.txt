﻿cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("SimpleFSR1")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build as a shared library" OFF)

if(MSVC)
  set(CMAKE_GENERATOR_PLATFORM x64)
endif()

include_directories("Src/include")

add_library(SimpleFSR1 "Src/source/SimpleFSR1.cpp" "Src/include/SimpleFSR1.h" "Src/include/FSR1.h")

target_include_directories(SimpleFSR1 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Src/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(SimpleFSR1 PRIVATE PRIVATEd3d11.lib dxgi.lib dxguid.lib d3dcompiler.lib)

install(TARGETS SimpleFSR1
  EXPORT SimpleFSR1Targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include)

install(EXPORT SimpleFSR1Targets
  FILE SimpleFSR1Targets.cmake
  NAMESPACE SimpleFSR1::
  DESTINATION lib/cmake/SimpleFSR1)

install(DIRECTORY include/
  DESTINATION include)

MESSAGE(STATUS "Generating shaders header file...")

set(HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Src/include/FSR1.h")
set(SHADERS_NAME
  fsr1.hlsl
)

set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shader")
file(WRITE ${HEADER_FILE} "#pragma once\n\n")
file(APPEND ${HEADER_FILE} "// Auto-generated file: Do not modify\n\n")
file(APPEND ${HEADER_FILE} "namespace DefaultShader {\n")

foreach(SHADER ${SHADERS_NAME})
  file(READ "${SHADERS_DIR}/${SHADER}" SHADER_CONTENT)

  # Remove the ".hlsl" extension from the shader name
  string(REGEX REPLACE ".hlsl" "" SHADER ${SHADER})

  file(APPEND ${HEADER_FILE} "static const char *${SHADER} = R\"(\n")
  file(APPEND ${HEADER_FILE} "${SHADER_CONTENT}\n")
  file(APPEND ${HEADER_FILE} ")\";\n")
endforeach()

file(APPEND ${HEADER_FILE} "}\n\n")
MESSAGE(STATUS "Shaders header file generated")