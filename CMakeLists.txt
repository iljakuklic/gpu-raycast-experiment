
cmake_minimum_required(VERSION 2.6)

project(local_raycast_demo)

include(FindOpenGL)
include(FindGLUT)
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

set(srcs main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${srcs})
target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_glut_LIBRARY} GLEW png)

