# - A tiny wrapper around the FindLibXml2.cmake macro that comes with CMake.
# Its purpose is to set some variables that should have been set according
# to the recommendations for writing FindXXX.cmake files.

# Call the "real" FindLibXml2 module.
include(${CMAKE_ROOT}/Modules/FindLibXml2.cmake)

# Set missing variables if LibXml2 was found.
if(LIBXML2_FOUND)
  if(NOT DEFINED LIBXML2_INCLUDE_DIRS)
    set(LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR})
  endif(NOT DEFINED LIBXML2_INCLUDE_DIRS)
endif(LIBXML2_FOUND)
