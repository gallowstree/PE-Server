cmake_minimum_required(VERSION 3.2)
project(test_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCE_FILES main.cpp Player.cpp Player.h serialization.h command.h Projectile.cpp Projectile.h reliable_message.h)
add_executable(test_server ${SOURCE_FILES} main.cpp)
TARGET_LINK_LIBRARIES(test_server pthread)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
find_package(SFML REQUIRED system window graphics audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(test_server ${SFML_LIBRARIES})
endif()
