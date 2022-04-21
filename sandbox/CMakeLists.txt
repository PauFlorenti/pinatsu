
# Game executable

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sandbox.cpp)

#FILE(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/ SRC)
#message(${SRC})

include_directories(${PROJECT_SOURCE_DIR}/engine/src)

add_definitions(-WX)
add_definitions(-Zi)
add_definitions(-DDEBUG)

add_executable(sandbox ${SRC})
target_link_libraries(sandbox PUBLIC engine user32.lib ${Vulkan_LIBRARIES})