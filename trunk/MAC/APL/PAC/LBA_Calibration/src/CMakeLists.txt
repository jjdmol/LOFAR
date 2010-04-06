# $Id: CMakeLists.txt 15028 2010-02-19 09:18:56Z donker $

include(LofarMatlab)
include(LofarPackageVersion)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location.
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX APL/${PACKAGE_NAME}
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Add C++ shared library generated from Matlab M-files
lofar_add_matlab_library(lba_calibration lba_calibration.m)

#lofar_add_library(lba_calibration Package__Version.cc)
lofar_add_bin_program(versionlba_calibration
  versionlba_calibration.cc
  Package__Version.cc)

