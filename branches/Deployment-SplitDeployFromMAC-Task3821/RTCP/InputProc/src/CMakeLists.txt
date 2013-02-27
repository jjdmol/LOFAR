include(LofarPackageVersion)

lofar_add_library(inputproc
  Package__Version.cc
  BufferSettings.cc
  Generator.cc
  PacketReader.cc
  Ranges.cc
  SharedMemory.cc
  StationID.cc
  RSPBoards.cc
  )

lofar_add_bin_program(newInputSection newInputSection.cc)
lofar_add_bin_program(filterRSP filterRSP.cc)
