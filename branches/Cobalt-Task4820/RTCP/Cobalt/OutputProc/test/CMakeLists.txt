# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tWriter tWriter.cc)
endif()

lofar_add_test(tMeasurementSetFormat tMeasurementSetFormat.cc)
lofar_add_test(tMSWriterDAL tMSWriterDAL.cc)
# The MS Writer needs the LOFAR Storage manager and static meta data files.
# For production, LOFARROOT is used, but build/source dir structure differs
# from installed dir. For regression tests, have cmake provide paths.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tMSWriterCorrelated.run.in
  ${CMAKE_CURRENT_BINARY_DIR}/tMSWriterCorrelated.run @ONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tMSWriterCorrelated.parset.in
  ${CMAKE_CURRENT_BINARY_DIR}/tMSWriterCorrelated.parset @ONLY)
lofar_add_test(tMSWriterCorrelated tMSWriterCorrelated.cc)
lofar_add_test(tDAL tDAL.cc)
lofar_add_test(tFastFileStream tFastFileStream.cc)
lofar_add_test(tTBB_StaticMapping tTBB_StaticMapping.cc)
#lofar_add_test(tTBB_Writer)

