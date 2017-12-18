cmake_minimum_required(VERSION 3.8)
project(MusicDR C)

set(CMAKE_C_STANDARD 11)

include_directories(head)

set (EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_SOURCE_DIR}/link/libCL_Common.a)

add_executable(MusicDR head main.c)

#@see https://github.com/WakelessDragon/CL_Common
target_link_libraries (MusicDR  ${EXTRA_LIBS})