INSTHDRS         =                                \
DFTAll.h \
DH_Solution.h                                   \
DH_WorkOrder.h                                  \
WH_Evaluate.h                                   \
WH_PSS3.h                                       \
BlackBoardDemo.h                                \
Quality.h                                       \
Strategy.h                                      \
StrategyImpl.h                                  \
SI_Peeling.h                                    \
SI_Simple.h                                     \
SI_WaterCal.h                                   \
SI_Randomized.h 				\
MeqCalibraterImpl.h                             \
CalibratorOld.h                                 \
Calibrator.h					\
TryOut.h \
MNS/MeqDomain.h \
MNS/MeqExpr.h \
MNS/MeqHist.h \
MNS/MeqJonesExpr.h \
MNS/MeqJonesNode.h \
MNS/MeqLofarPoint.h \
MNS/MeqLofarStatSources.h \
MNS/MeqMatrixComplexArr.h \
MNS/MeqMatrixComplexSca.h \
MNS/MeqMatrix.h \
MNS/MeqMatrixRealArr.h \
MNS/MeqMatrixRealSca.h \
MNS/MeqMatrixRep.h \
MNS/MeqMatrixTmp.h \
MNS/MeqParm.h \
MNS/MeqParmHolder.h \
MNS/MeqParmPolc.h \
MNS/MeqParmSingle.h \
MNS/MeqPhaseRef.h \
MNS/MeqPointDFT.h \
MNS/MeqPointSource.h \
MNS/MeqPointSourceList.h \
MNS/MeqPolc.h \
MNS/MeqRequest.h \
MNS/MeqRequestId.h \
MNS/MeqResult.h \
MNS/MeqSourceList.h \
MNS/MeqStatExpr.h \
MNS/MeqStation.h \
MNS/MeqStatSources.h \
MNS/MeqStatUVW.h \
MNS/MeqStoredParmPolc.h \
MNS/MeqUVWPolc.h \
MNS/MeqWsrtInt.h \
MNS/MeqWsrtPoint.h \
MNS/ParmTableAIPS.h \
MNS/ParmTable.h

NOINSTHDRS =

TCCHDRS =

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES = libpss3.la

libpss3_la_SOURCES = \
DFTAll.cc \
Simulate.cc                                             \
DH_Solution.cc                                          \
DH_WorkOrder.cc                                         \
WH_Evaluate.cc                                          \
WH_PSS3.cc                                              \
Quality.cc                                              \
Strategy.cc                                             \
StrategyImpl.cc                                         \
SI_Peeling.cc                                           \
SI_Simple.cc                                            \
SI_WaterCal.cc                                          \
SI_Randomized.cc					\
MeqCalibraterImpl.cc                                    \
CalibratorOld.cc 	                                \
Calibrator.cc						\
TryOut.cc \
MNS/MeqDomain.cc \
MNS/MeqExpr.cc \
MNS/MeqHist.cc \
MNS/MeqJonesExpr.cc \
MNS/MeqJonesNode.cc \
MNS/MeqLofarPoint.cc \
MNS/MeqLofarStatSources.cc \
MNS/MeqMatrix.cc \
MNS/MeqMatrixComplexArr.cc \
MNS/MeqMatrixComplexSca.cc \
MNS/MeqMatrixRealArr.cc \
MNS/MeqMatrixRealSca.cc \
MNS/MeqMatrixRep.cc \
MNS/MeqMatrixTmp.cc \
MNS/MeqParm.cc \
MNS/MeqParmPolc.cc \
MNS/MeqParmSingle.cc \
MNS/MeqPhaseRef.cc \
MNS/MeqPointDFT.cc \
MNS/MeqPointSource.cc \
MNS/MeqPolc.cc \
MNS/MeqRequest.cc \
MNS/MeqResult.cc \
MNS/MeqSourceList.cc \
MNS/MeqStatExpr.cc \
MNS/MeqStation.cc \
MNS/MeqStatSources.cc \
MNS/MeqStatUVW.cc \
MNS/MeqStoredParmPolc.cc \
MNS/MeqUVWPolc.cc \
MNS/MeqWsrtInt.cc \
MNS/MeqWsrtPoint.cc \
MNS/ParmTableAIPS.cc \
MNS/ParmTable.cc



bin_PROGRAMS = BlackBoardDemo

BlackBoardDemo_SOURCES = BlackBoardDemo.cc

BlackBoardDemo_LDADD = libpss3.la
BlackBoardDemo_DEPENDENCIES = libpss3.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common
