
# Find source files for engine
#some shiet
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/containers/ SRC_CONTAINERS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/core/ SRC_CORE)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/memory/ SRC_MEMORY)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ SRC_PLATFORM)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/ SRC_RENDERER)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/ SRC_VULKAN)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/shaders/ SRC_VULKAN_SHADERS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/resources/loaders/ SRC_RESOURCES)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/systems/ SRC_SYSTEMS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/systems/components/ SRC_COMPONENTS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/systems/handle/ SRC_HANDLE)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/systems/entity/ SRC_ENTITY)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/systems/modules/ SRC_MODULES)

# Include necessary directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${Vulkan_INCLUDE_DIRS})

# Warning compiler omissions
add_definitions(-wd4805)
add_definitions(-wd4267)
add_definitions(-wd4996)
add_definitions(-wd4244)
add_definitions(-wd4305)
add_definitions(-WX)
add_definitions(-Zi)
add_definitions(-DDEBUG)

# Generate Engine library
add_library(engine  STATIC 
                    ${SRC_CONTAINERS}
                    ${SRC_CORE}
                    #${SRC_EXTERNALS} -- In case we need some external cpp compiled ...
                    ${SRC_PLATFORM}
                    ${SRC_RENDERER}
                    ${SRC_MEMORY}
                    ${SRC_RESOURCES}
                    ${SRC_COMPONENTS}
                    ${SRC_HANDLE}
                    ${SRC_ENTITY}
                    ${SRC_MODULES}
                    ${SRC_RENDERER}
                    ${SRC_VULKAN}
                    ${SRC_VULKAN_SHADERS}
                    ${SRC_SYSTEMS})

# Build imgui library
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/ SRC_IMGUI)
add_library(imgui STATIC ${SRC_IMGUI})

# Set precompiled headers
set(PRE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/pnt_platform.h)

target_precompile_headers(engine PUBLIC ${PRE_HEADERS})
target_precompile_headers(imgui PUBLIC ${PRE_HEADERS})