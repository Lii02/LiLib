cmake_minimum_required(VERSION 4.0)
project(liLib VERSION 1.0.0 LANGUAGES CXX)

option(LILIB_BUILD_TESTS "Build the liLib unit tests" ${PROJECT_IS_TOP_LEVEL})

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Language standard and warning flags shared by every target in this project
function(lilib_set_target_defaults target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endfunction()

find_package(Threads REQUIRED)

# CONFIGURE_DEPENDS re-runs the glob at build time, so added or removed files are picked up automatically
file(GLOB_RECURSE LILIB_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)

add_library(liLib STATIC ${LILIB_SOURCES})
add_library(LiLib::LiLib ALIAS liLib)

lilib_set_target_defaults(liLib)

# The public headers need C++17, so consumers must compile with it too
target_compile_features(liLib PUBLIC cxx_std_17)

# Consumers include headers as "LiLib/Strings/String.h". Keep headers inside src/LiLib/: directly in src/,
# case-insensitive filesystems would resolve system headers such as <string.h> to String.h.
target_include_directories(liLib PUBLIC src)

target_link_libraries(liLib PUBLIC Threads::Threads)

if(LILIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
