cmake_minimum_required(VERSION 2.8)

project( picamdemo )

SET(COMPILE_DEFINITIONS -Werror)

#include required packages
find_package( OpenCV REQUIRED )
find_package( Threads REQUIRED )
find_package( PkgConfig REQUIRED )

# OpenCV
include_directories(${OpenCV_INCLUDE_DIR})

# MMAL
pkg_search_module(MMAL REQUIRED mmal)
include_directories(${MMAL_INCLUDE_DIRS})
link_directories(${MMAL_LIBRARY_DIRS} )

# EGL
pkg_search_module(EGL REQUIRED egl)
include_directories(${EGL_INCLUDE_DIRS})
link_directories(${EGL_LIBRARY_DIRS} )

# Userland
include_directories(${USERLAND_DIR})

# Projectdirs
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/shaders)

add_executable(picamdemo 
    picam.cpp 
    camera.cpp 
    cameracontrol.cpp 
    graphics.cpp)

target_link_libraries(picamdemo ${OpenCV_LIBS})
target_link_libraries(picamdemo ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(picamdemo ${MMAL_LIBRARIES})
target_link_libraries(picamdemo ${EGL_LIBRARIES})
