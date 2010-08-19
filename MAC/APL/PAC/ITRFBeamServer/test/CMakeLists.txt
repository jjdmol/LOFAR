# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

include(LofarCTest)

# Add current binary directory to -I path, so that the generated C++ header
# files can be found.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Tests cannot be run at this stage, because they all need to connect to a
# Service Broker, which has not been built yet. Therefore we will only build
# the test programs.
lofar_add_executable(tLBATracking tLBATracking.cc ../src/J2000Converter.cc)
lofar_add_executable(tAnaBeamMgr tAnaBeamMgr.cc ../src/AnaBeamMgr.cc ../src/Beam.cc ../src/AnalogueBeam.cc ../src/J2000Converter)
lofar_add_executable(tJ2000Converter tJ2000Converter.cc
../src/J2000Converter.cc) 
