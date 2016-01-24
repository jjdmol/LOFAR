# $Id: CMakeLists.txt 14588 2009-12-02 15:31:07Z overeem $

include(LofarCTest)

# Add current binary directory to -I path, so that the generated C++ header
# files can be found.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Tests cannot be run at this stage, because they all need to connect to a
# Service Broker, which has not been built yet. Therefore we will only build
# the test programs.
lofar_add_executable(tCR_Protocol tCR_Protocol.cc CR_Protocol.cc)

