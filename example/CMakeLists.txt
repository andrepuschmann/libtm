########################################################################
# Add executables and link to libraries
########################################################################
ADD_EXECUTABLE (benchmark benchmark.cpp)
TARGET_LINK_LIBRARIES (benchmark tm boost_thread)
