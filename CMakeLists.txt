cmake_minimum_required(VERSION 3.8)
project(3dbigproject)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCES "./*.cpp")

add_executable(3dbigproject ${SOURCES})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

# Find SOIL
# Find the SOIL includes and library
#
#  SOIL_INCLUDE_DIRS - where to find SOIL.h, etc.
#  SOIL_LIBRARIES    - List of libraries when using SOIL.
#  SOIL_FOUND        - True if SOIL found.
#
# Based on the FindZLIB.cmake module.

IF (SOIL_INCLUDE_DIR)
    # Already in cache, be silent
    SET(SOIL_FIND_QUIETLY TRUE)
ENDIF (SOIL_INCLUDE_DIR)

FIND_PATH(SOIL_INCLUDE_DIR SOIL.h PATH_SUFFIXES include/SOIL include)

SET(SOIL_NAMES SOIL Soil soil)
FIND_LIBRARY(SOIL_LIBRARY NAMES ${SOIL_NAMES} )
MARK_AS_ADVANCED( SOIL_LIBRARY SOIL_INCLUDE_DIR )

# Per-recommendation
SET(SOIL_INCLUDE_DIRS "${SOIL_INCLUDE_DIR}")
SET(SOIL_LIBRARIES    "${SOIL_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set SOIL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SOIL DEFAULT_MSG SOIL_LIBRARIES SOIL_INCLUDE_DIRS)





target_link_libraries(3dbigproject ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${SOIL_LIBRARY})
#inspired from https://stackoverflow.com/questions/9460242/how-to-compile-glut-opengl-project-with-cmake-and-kdevelop-in-linux
