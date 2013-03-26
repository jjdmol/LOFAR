include(LofarPackageVersion)

# Add current source directory to -I path. This is needed because InputProc uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(inputproc
  Package__Version.cc
  RSPBoards.cc
  Buffer/BufferSettings.cc
  Buffer/Ranges.cc
  Buffer/SharedMemory.cc
  Buffer/StationID.cc
  Station/Generator.cc
  Station/PacketFactory.cc
  Station/PacketReader.cc
  Station/PacketsToBuffer.cc
  Transpose/MPISendStation.cc
  Transpose/MPIReceiveStations.cc
  Transpose/MPIUtil.cc
  )

lofar_add_bin_program(filterRSP Station/filterRSP.cc)
lofar_add_bin_program(printRSP Station/printRSP.cc)
