include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(inputproc
  Package__Version.cc
  RSPBoards.cc
  Buffer/BufferSettings.cc
  Buffer/Ranges.cc
  Buffer/SampleBuffer.cc
  Buffer/SharedMemory.cc
  Buffer/StationID.cc
  Delays/Delays.cc
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
lofar_add_bin_program(generate Station/generate.cc)
lofar_add_bin_program(capture Station/capture.cc)
