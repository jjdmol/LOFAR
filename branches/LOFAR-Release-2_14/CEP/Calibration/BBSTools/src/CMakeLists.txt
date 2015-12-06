# $Id: CMakeLists.txt 18231 2011-06-09 14:46:14Z duscha $

include(LofarPackageVersion)

lofar_add_bin_program(addUV2MS addUV2MS.cc)
lofar_add_bin_program(patchCasaFreq patchCasaFreq.cc)

# Suppress compiler warnings from casarest by disabling them for some sources
set_source_files_properties(
  addUV2MS.cc
  PROPERTIES COMPILE_FLAGS "-Wno-ignored-qualifiers -Wno-overloaded-virtual"
)

