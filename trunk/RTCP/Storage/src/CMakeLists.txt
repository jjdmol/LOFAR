# $Id$

include(LofarPackageVersion)

set(storage_LIB_SRCS
  Package__Version.cc
  InputThread.cc
  SubbandWriter.cc
  MSWriter.cc
  MSWriterCasa.cc
  MSWriterNull.cc
  MSWriterFile.cc)

lofar_add_library(storage ${storage_LIB_SRCS})

lofar_add_executable(Storage_main Storage_main.cc)
lofar_add_executable(versionstorage versionstorage.cc)
