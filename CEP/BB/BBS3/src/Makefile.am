pkginclude_HEADERS = \
DH_Solution.h                                   \
DH_WOSolve.h					\
DH_WOPrediff.h					\
DH_Prediff.h					\
StrategyController.h				\
WH_Control.h                                    \
WH_Prediff.h					\
WH_Solve.h					\
BlackBoardDemo.h                                \
Quality.h                                       \
SC_Simple.h					\
SC_WritePredData.h				\
SC_CompoundIter.h				\
MMap.h						\
FlagsMap.h					\
Prediffer.h					\
Solver.h					\
ParmData.h					\
ParmWriter.h					\
BBSTestLogger.h

pkginclude_MNSdir = $(pkgincludedir)/MNS
pkginclude_MNS_HEADERS = \
MNS/MeqBaseDFTPS.h \
MNS/MeqBaseLinPS.h \
MNS/MeqDFTPS.h \
MNS/MeqDomain.h \
MNS/MeqExpr.h \
MNS/MeqFunklet.h \
MNS/MeqJonesCMul2.h \
MNS/MeqJonesCMul3.h \
MNS/MeqJonesExpr.h \
MNS/MeqJonesInvert.h \
MNS/MeqJonesMul2.h \
MNS/MeqJonesMul3.h \
MNS/MeqJonesNode.h \
MNS/MeqJonesResult.h \
MNS/MeqJonesSum.h \
MNS/MeqLMN.h \
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
MNS/MeqPointSource.h \
MNS/MeqPolc.h \
MNS/MeqRequest.h \
MNS/MeqRequestId.h \
MNS/MeqResult.h \
MNS/MeqResultVec.h \
MNS/MeqSourceList.h \
MNS/MeqStatExpr.h \
MNS/MeqStation.h \
MNS/MeqStatUVW.h \
MNS/MeqStoredParmPolc.h

noinst_HEADERS =


lib_LTLIBRARIES = libbbs3.la

libbbs3_la_SOURCES  = 					\
DH_Solution.cc                                          \
DH_WOSolve.cc						\
DH_WOPrediff.cc						\
DH_Prediff.cc						\
StrategyController.cc					\
WH_Control.cc                                           \
WH_Prediff.cc						\
WH_Solve.cc						\
BlackBoardDemo.cc                                       \
Quality.cc                                              \
SC_Simple.cc						\
SC_WritePredData.cc					\
SC_CompoundIter.cc					\
MMap.cc							\
FlagsMap.cc						\
Prediffer.cc						\
Solver.cc						\
ParmData.cc						\
ParmWriter.cc						\
MNS/MeqBaseDFTPS.cc \
MNS/MeqBaseLinPS.cc \
MNS/MeqDFTPS.cc \
MNS/MeqDomain.cc \
MNS/MeqExpr.cc \
MNS/MeqFunklet.cc \
MNS/MeqJonesCMul2.cc \
MNS/MeqJonesCMul3.cc \
MNS/MeqJonesExpr.cc \
MNS/MeqJonesInvert.cc \
MNS/MeqJonesMul2.cc \
MNS/MeqJonesMul3.cc \
MNS/MeqJonesNode.cc \
MNS/MeqJonesSum.cc \
MNS/MeqLMN.cc \
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
MNS/MeqPointSource.cc \
MNS/MeqPolc.cc \
MNS/MeqRequest.cc \
MNS/MeqResult.cc \
MNS/MeqResultVec.cc \
MNS/MeqSourceList.cc \
MNS/MeqStatExpr.cc \
MNS/MeqStation.cc \
MNS/MeqStatUVW.cc \
MNS/MeqStoredParmPolc.cc \
BBSTestLogger.cc


#bin_PROGRAMS = BlackBoardDemo fillsqldb
bin_PROGRAMS = BBS3 psReader

psReader_SOURCES = ParameterSetReader.cc

BBS3_SOURCES = BBS3.cc
BBS3_LDADD = libbbs3.la
BBS3_DEPENDENCIES = libbbs3.la $(LOFAR_DEPEND)

scriptdir = $(bindir)
dist_script_SCRIPTS = clearBB

include $(top_srcdir)/Makefile.common
