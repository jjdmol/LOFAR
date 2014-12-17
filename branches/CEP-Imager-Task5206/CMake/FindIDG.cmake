# - Try to find IDG
# Once done, this will define
#
#  IDG_FOUND - system has IDG
#  IDG_INCLUDE_DIRS - the IDG include directories
#  IDG_LIBRARIES - link these to use IDG

include(LibFindMacros)

# Dependencies
# libfind_package(IDG FFTW3)

# Include dir
find_path(IDG_INCLUDE_DIR
  NAMES idg/Proxies.h
)

# Finally the library itself
find_library(IDG_LIBRARY
  NAMES idg
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.

# set(FFTW3_INCLUDE_DIRS FFTW3_INCLUDE_DIR)
# set(FFTW3_LIBRARIES FFTW3_LIBRARY)

set(IDG_PROCESS_INCLUDES IDG_INCLUDE_DIR)
set(IDG_PROCESS_LIBS IDG_LIBRARY)
libfind_process(IDG)
