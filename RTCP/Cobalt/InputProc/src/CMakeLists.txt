include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(_inputproc_sources
  Package__Version.cc
  RSPBoards.cc
  RSPTimeStamp.cc
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
  Station/RSPPacketFactory.cc
)

message(STATUS "**** MPI_FOUND = ${MPI_FOUND}")
message(STATUS "**** USE_MPI = ${USE_MPI}")
message(STATUS "**** HAVE_MPI = ${HAVE_MPI}")

if(MPI_FOUND)
  list(APPEND _inputproc_sources
    Transpose/MPISendStation.cc
    Transpose/MPIReceiveStations.cc
    Transpose/MPIUtil.cc
  )
endif(MPI_FOUND)

lofar_add_library(inputproc ${_inputproc_sources})
if(UNIX AND NOT APPLE)
  target_link_libraries(inputproc rt)
endif(UNIX AND NOT APPLE)

lofar_add_bin_program(filterRSP Station/filterRSP.cc)
lofar_add_bin_program(printRSP Station/printRSP.cc)
lofar_add_bin_program(generateRSP Station/generateRSP.cc)
lofar_add_bin_program(generate Station/generate.cc)
lofar_add_bin_program(capture Station/capture.cc)
lofar_add_bin_program(printDelays Delays/printDelays.cc)

# install the logprop file
install(FILES
  Delays/printDelays.log_prop
  DESTINATION etc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mpirun.sh.in
  ${CMAKE_BINARY_DIR}/bin/mpirun.sh @ONLY)

install(PROGRAMS
  ${CMAKE_BINARY_DIR}/bin/mpirun.sh
  DESTINATION bin)
