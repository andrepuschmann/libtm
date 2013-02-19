########################################################################
# Add executables and link to libraries
########################################################################
INCLUDE_DIRECTORIES (${LIBTM_SOURCE_DIR}/src)
ADD_EXECUTABLE (benchmark benchmark.cpp)
TARGET_LINK_LIBRARIES (benchmark tm rt boost_thread)
