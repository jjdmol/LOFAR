# $Id$

include(LofarCTest)

lofar_add_test(tMeasurementSetFormat tMeasurementSetFormat.cc)
lofar_add_test(tMSWriterDAL tMSWriterDAL.cc)
# The MS Writer needs to locate static meta data files.
# For production, LOFARROOT is used, but the source dir structure is different.
# So for regression tests, pass the source dir.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tMSWriterCorrelated.parset.in
  ${CMAKE_CURRENT_BINARY_DIR}/tMSWriterCorrelated.parset @ONLY)
lofar_add_test(tMSWriterCorrelated tMSWriterCorrelated.cc)
lofar_add_test(tDAL tDAL.cc)
lofar_add_test(tFastFileStream tFastFileStream.cc)
#lofar_add_test(tAH_TestStorage tAH_TestStorage.cc)
#lofar_add_test(tTBB_StaticMapping tTBB_StaticMapping.cc)
#lofar_add_test(tTBB_Writer)

