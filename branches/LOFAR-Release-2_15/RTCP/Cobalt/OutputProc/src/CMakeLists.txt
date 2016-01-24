# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

#add_definitions(-DTBB_DUMP_RAW_STATION_FRAMES)

lofar_add_library(outputproc 
  Package__Version.cc
  FastFileStream.cc
  GPUProcIO.cc
  InputThread.cc
  MSWriter.cc
  MSWriterCorrelated.cc
  MSWriterDAL.cc
  MSWriterNull.cc
  MSWriterFile.cc
  MeasurementSetFormat.cc
  CommonLofarAttributes.cc
  OutputThread.cc
  SubbandWriter.cc
  TBB_StaticMapping.cc
)

lofar_add_sbin_scripts(gnuplotMS.sh)

install(FILES
  outputProc.log_prop
  TBB_Writer.log_prop
  DESTINATION etc)

lofar_add_bin_program(outputProc outputProc.cc)
lofar_add_bin_program(versionoutputproc versionoutputproc.cc)
lofar_add_bin_program(TBB_Writer TBB_Writer_main.cc TBB_Writer.cc)

lofar_add_sbin_program(createHeaders createHeaders.cc)
lofar_add_sbin_program(plotMS plotMS.cc)

