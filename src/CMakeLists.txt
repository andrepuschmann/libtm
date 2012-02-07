########################################################################
# Build the library from source files
########################################################################
SET(libtm_sources
    tm.cpp
)

# Static library (test code can link against this)
ADD_LIBRARY(tm SHARED ${libtm_sources})
