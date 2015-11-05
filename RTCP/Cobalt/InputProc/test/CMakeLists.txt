# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tRanges tRanges.cc)

  lofar_add_test(tDelays tDelays.cc)
  lofar_add_test(tRSPTimeStamp tRSPTimeStamp.cc)
endif(UNITTEST++_FOUND)

lofar_add_test(tRSP tRSP.cc)
lofar_add_test(tPacketReader tPacketReader.cc)
lofar_add_test(tPacketFactory tPacketFactory.cc)
lofar_add_test(tGenerator tGenerator.cc)
lofar_add_test(t_generateRSP t_generateRSP.cc)

if(UNITTEST++_FOUND)
  lofar_add_test(tMPIUtil2 tMPIUtil2.cc)
endif(UNITTEST++_FOUND)

if(MPI_FOUND)
  lofar_add_test(tMPI tMPI.cc)
endif(MPI_FOUND)
