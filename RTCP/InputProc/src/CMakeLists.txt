include(LofarPackageVersion)

lofar_add_library(inputproc
  Package__Version.cc
  BufferSettings.cc
  Generator.cc
  SharedMemory.cc
  StationID.cc
  StationStreams.cc
  )

lofar_add_bin_program(newInputSection newInputSection.cc)
