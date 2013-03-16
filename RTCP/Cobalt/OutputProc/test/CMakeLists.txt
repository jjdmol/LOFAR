# $Id$

include(LofarCTest)

lofar_add_test(tMeasurementSetFormat tMeasurementSetFormat.cc)
lofar_add_test(tMSWriterDAL tMSWriterDAL.cc)
lofar_add_test(tMSWriterCorrelated tMSWriterCorrelated.cc)
lofar_add_test(tDAL tDAL.cc)
lofar_add_test(tFastFileStream tFastFileStream.cc)
#lofar_add_test(tAH_TestStorage tAH_TestStorage.cc)
lofar_add_test(tTBB_StaticMapping tTBB_StaticMapping.cc)
#lofar_add_test(tTBB_Writer)

