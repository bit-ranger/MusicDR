cmake_minimum_required(VERSION 3.8)
project(MusicDR C)

set(CMAKE_C_STANDARD 11)

include_directories(head)

set (EXTRA_LIBS ${EXTRA_LIBS})

add_executable(MusicDR main.c)

#@see https://github.com/WakelessDragon/CL_Common
target_link_libraries (MusicDR  ${EXTRA_LIBS} /Doc/MyRepo/CL-Common/cmake-build-debug/libCL_Common.a)
