########################################################################
# Prevent in-tree builds
########################################################################
IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "Prevented in-tree built. This is bad practice.")
ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

########################################################################
# Project setup
#######################################################################
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (LIBTM)
ENABLE_TESTING()
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (example)

# Make sure the compiler can find include files
include_directories (${LIBTM_SOURCE_DIR}/src) 

# Make sure the linker can find the library once it is built. 
link_directories (${LIBTM_BINARY_DIR}/src) 
