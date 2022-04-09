cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Dijiang)
set(CMAKE_CXX_STANDARD 20)
# 获取OS信息
if (CMAKE_HOST_WIN32)
	MESSAGE(STATUS "WIN32")
	set(__win32__ 1)
elseif(CMAKE_HOST_APPLE)
	MESSAGE(STATUS "APPLE")
	add_compile_options(-Wno-nullability-completeness)
	set(__macosx__ 1)
elseif(CMAKE_HOST_UNIX)
	MESSAGE(STATUS "UNIX")
	set(__linux__ 1)
endif()

add_compile_options(-g3)


project(${PROJECT_NAME})

include_directories(./src/)
include_directories(./src/render)
include_directories(./src/render/rhi)
include_directories(./src/render/vulkan)
include_directories(./deps/)

aux_source_directory(./src/ SOURCES)
aux_source_directory(./deps/ SOURCES)
aux_source_directory(./src/utils SOURCES)
aux_source_directory(./src/render SOURCES)
aux_source_directory(./src/resource SOURCES)
aux_source_directory(./src/render/rhi SOURCES)
aux_source_directory(./src/render/vulkan SOURCES)

add_executable(${PROJECT_NAME} ${SOURCES})

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND mkdir -p shaders
	COMMAND ../tools/shaderc.sh ../shaders ./shaders
)


find_package(Eigen3 REQUIRED FATAL_ERROR)
find_package(glfw3 REQUIRED FATAL_ERROR)
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(assimp REQUIRED FATAL_ERROR)
find_package(yaml-cpp REQUIRED FATAL_ERROR)
if (Eigen3_FOUND)
	include_directories(${EIGEN3_INCLUDE_DIRS})
endif(Eigen3_FOUND)
if (glfw3_FOUND)
	include_directories(${glfw3_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} glfw)
endif(glfw3_FOUND)
if (Vulkan_FOUND)
	include_directories(${Vulkan_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
endif(Vulkan_FOUND)
if (assimp_FOUND)
	include_directories(${ASSIMP_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
endif(assimp_FOUND)
if (yaml-cpp_FOUND)
	include_directories(${YAML_CPP_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${YAML_CPP_LIBRARIES})
endif(yaml-cpp_FOUND)

set(SPIRV ${PROJECT_SOURCE_DIR}/deps/spirv-cross)
include_directories(${SPIRV})

MESSAGE(STATUS "...${SPIRV}")

target_link_libraries(${PROJECT_NAME}
	${SPIRV}/libspirv-cross.a
	)

