# $Id: CMakeLists.txt 14466 2009-11-17 08:58:28Z schoenmakers $

include(LofarCTest)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

include_directories(/usr/local/matlab-2008a/extern/include)

lofar_add_test(tCalibrate tCalibrate.cc)
target_link_libraries(tCalibrate 
  ${CMAKE_CURRENT_BINARY_DIR}/../src/liblba_calibration.so
  /usr/local/matlab-2008a/bin/glnx86/libmwmclmcrrt.so)
