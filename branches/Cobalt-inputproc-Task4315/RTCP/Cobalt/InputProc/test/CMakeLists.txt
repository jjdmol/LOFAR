# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

lofar_add_test(tRanges tRanges.cc)
lofar_add_test(tSharedMemory tSharedMemory.cc)
lofar_add_test(tSampleBuffer tSampleBuffer.cc)
if(UNITTEST++_FOUND)
  lofar_add_test(tSampleBufferSync tSampleBufferSync.cc)
endif(UNITTEST++_FOUND)

lofar_add_test(tRSP tRSP.cc)
lofar_add_test(tRSPTimeStamp2 tRSPTimeStamp2.cc)
lofar_add_test(tPacketReader tPacketReader.cc)
lofar_add_test(tPacketFactory tPacketFactory.cc)
lofar_add_test(tGenerator tGenerator.cc)

lofar_add_test(tPacketWriter tPacketWriter.cc)
lofar_add_test(tPacketsToBuffer tPacketsToBuffer.cc)

if(UNITTEST++_FOUND)
  lofar_add_test(tBlockReader tBlockReader.cc)
  lofar_add_test(tMPISendReceiveStation tMPISendReceiveStation.cc)
endif(UNITTEST++_FOUND)

lofar_add_test(tMPITransfer tMPITransfer.cc)
