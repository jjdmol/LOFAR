# $Id: CMakeLists.txt 14466 2009-11-17 08:58:28Z schoenmakers $

include(LofarCTest)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_test(tCalibrate tCalibrate.cc DEPENDS lba_calibration)
