# $Id$

include(LofarPackageVersion)

# Extra preprocessor definitions
add_definitions(-DUSES_NEW_IOSTREAMS -DLOFARLOGGER_SUBPACKAGE="PVSS")

# Add current source directory to -I path. This is needed because GCFPVSS uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(gcfpvss
  Package__Version.cc
  GCF_PValue.cc
  GCF_PVChar.cc
  GCF_PVBool.cc
  GCF_PVDateTime.cc
  GCF_PVDouble.cc
  GCF_PVString.cc
  GCF_PVInteger.cc
  GCF_PVUnsigned.cc
  GCF_PVDynArr.cc
  GCF_PVBlob.cc
  GSA_SCADAHandler.cc
  GSA_PvssApi.cc
  GSA_Resources.cc
  PVSSinfo.cc
  PVSSservice.cc)

lofar_add_bin_program(versiongcfpvss versiongcfpvss.cc)
