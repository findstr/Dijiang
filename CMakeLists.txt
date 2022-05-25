cmake_minimum_required(VERSION 3.22)
project(main)
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-g3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(ROOT_DIR "./")
set(DEPS_DIR "${ROOT_DIR}/deps")

include_directories(${DEPS_DIR}/stb)
include_directories(${DEPS_DIR}/lua)
include_directories(${DEPS_DIR}/spirv-cross)
include_directories(${DEPS_DIR}/vk_mem)

message(STATUS "build type:")
message(STATUS "CXX_FLAGS = ${CMAKE_CXX_FLAGS} ")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/")

add_subdirectory(deps/)
add_subdirectory(src/)

execute_process(COMMAND pwd)
execute_process(
	COMMAND mkdir -p shaders
	COMMAND ../tools/shaderc.sh ../shaders ./shaders
)


