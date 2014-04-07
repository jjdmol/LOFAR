# $Id$

include(LofarPackageVersion)

lofar_add_library(storage 
  Package__Version.cc
  InputThread.cc
  OutputThread.cc
  SubbandWriter.cc
  MSWriter.cc
  MSWriterNull.cc
  MSWriterFile.cc
  Format.cc
  MeasurementSetFormat.cc)

install(FILES
  Storage_main.log_prop
  DESTINATION etc)

lofar_add_bin_program(Storage_main Storage_main.cc)
lofar_add_bin_program(versionstorage versionstorage.cc)
