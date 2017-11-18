cmake_minimum_required(VERSION 3.8)
project(3dbigproject)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCES "./*.cpp" "./soil-master/src/*.c")

add_executable(3dbigproject ${SOURCES})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(3dbigproject ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
#inspired from https://stackoverflow.com/questions/60242/how-to-compile-glut-opengl-project-with-cmake-and-kdevelop-in-linux
