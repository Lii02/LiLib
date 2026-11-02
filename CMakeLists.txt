cmake_minimum_required(VERSION 3.16)
project(liLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

# CONFIGURE_DEPENDS re-runs the glob at build time, so added or removed files are picked up automatically
file(GLOB_RECURSE LILIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_library(liLib STATIC ${LILIB_SOURCES})
add_library(LiLib::LiLib ALIAS liLib)

target_compile_features(liLib PUBLIC cxx_std_17)

# Consumers include headers as "LiLib/Strings/String.h". Keep headers inside src/LiLib/: directly in src/,
# case-insensitive filesystems would resolve system headers such as <string.h> to String.h.
target_include_directories(liLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(liLib PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(liLib PRIVATE /W4)
else()
    target_compile_options(liLib PRIVATE -Wall -Wextra)
endif()
