cmake_minimum_required(VERSION 3.13)
project(PriorityQueue VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(
#        -Werror
        -pedantic-errors

        -Weffc++

        -Wall
        -Wextra
        -Wpedantic

        -Wcast-align
        -Wcast-qual
#        -Wconversion
        -Wctor-dtor-privacy
        -Wduplicated-branches
        -Wduplicated-cond
        -Wextra-semi
        -Wfloat-equal
        -Wlogical-op
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo

        -Wno-sign-compare
)

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_subdirectory(Generator)
add_subdirectory(Handler)
add_subdirectory(Scheduler)

add_custom_target(PriorityQueue)
add_dependencies(PriorityQueue generator_main)
add_dependencies(PriorityQueue handler_main)
add_dependencies(PriorityQueue scheduler_main)
