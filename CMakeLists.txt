cmake_minimum_required(VERSION 3.0.0)
project(Pinatsu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

# Find Vulkan
set(Vulkan_INCLUDE_DIRECTORIES "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")

find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan")
else()
    message(STATUS "Using Vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

message("Setting output at ${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(engine)
add_subdirectory(sandbox)