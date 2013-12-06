# $Id$

include(LofarCTest)

lofar_add_test(tMeasurementSetFormat tMeasurementSetFormat.cc)
lofar_add_test(tMSWriterDAL tMSWriterDAL.cc)
# The MS Writer needs the LOFAR Storage manager and static meta data files.
# For production, LOFARROOT is used, but build/source dir structure differs
# from installed dir. For regression tests, have cmake provide paths.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tMSWriterCorrelated_.run.in
  ${CMAKE_CURRENT_BINARY_DIR}/tMSWriterCorrelated_.run @ONLY)
lofar_add_test(tMSWriterCorrelated tMSWriterCorrelated.cc)
lofar_add_test(tDAL tDAL.cc)
lofar_add_test(tFastFileStream tFastFileStream.cc)
#lofar_add_test(tAH_TestStorage tAH_TestStorage.cc)
lofar_add_test(tTBB_StaticMapping tTBB_StaticMapping.cc)
#lofar_add_test(tTBB_Writer)

