
# Game executable

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sandbox.cpp)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/ SRC)

include_directories(${PROJECT_SOURCE_DIR}/engine/src)

add_definitions(-WX)
add_definitions(-Zi)
add_definitions(-DDEBUG)

add_executable(sandbox ${SRC})
target_link_libraries(sandbox PUBLIC engine imgui user32.lib ${Vulkan_LIBRARIES})

# Make linker to export all symbols from the library.
set_target_properties(sandbox PROPERTIES LINK_FLAGS "/WHOLEARCHIVE:engine")