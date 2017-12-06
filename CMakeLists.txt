cmake_minimum_required(VERSION 3.9)
project(MusicDR C)

set(CMAKE_C_STANDARD 11)

add_library(lib STATIC lib/HashMap.c)

include_directories(head)

set (EXTRA_LIBS ${EXTRA_LIBS} lib)

add_executable(MusicDR main.c)
target_link_libraries (MusicDR  ${EXTRA_LIBS})
