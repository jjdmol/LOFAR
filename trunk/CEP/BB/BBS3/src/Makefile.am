INSTHDRS         =                                \
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
MMap.h						\
FlagsMap.h					\
Prediffer.h					\
Solver.h					\
ParmData.h					\
ParmWriter.h					\
MNS/MeqBaseDFTPS.h \
MNS/MeqBaseLinPS.h \
MNS/MeqDFTPS.h \
MNS/MeqDomain.h \
MNS/MeqExpr.h \
MNS/MeqFunklet.h \
MNS/MeqHist.h \
MNS/MeqJonesCMul2.h \
MNS/MeqJonesExpr.h \
MNS/MeqJonesNode.h \
MNS/MeqJonesMul2.h \
MNS/MeqJonesMul3.h \
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
MNS/MeqPointSourceList.h \
MNS/MeqPolc.h \
MNS/MeqRequest.h \
MNS/MeqRequestId.h \
MNS/MeqResult.h \
MNS/MeqResultVec.h \
MNS/MeqSourceList.h \
MNS/MeqStatExpr.h \
MNS/MeqStation.h \
MNS/MeqStatUVW.h \
MNS/MeqStoredParmPolc.h \
MNS/ParmTableAIPS.h \
MNS/ParmTableBDB.h\
MNS/ParmTable.h

NOINSTHDRS =

TCCHDRS =

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES = libbbs3.la

libbbs3_la_SOURCES = \
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
MNS/MeqHist.cc \
MNS/MeqJonesCMul2.cc \
MNS/MeqJonesExpr.cc \
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
MNS/ParmTableAIPS.cc \
MNS/ParmTableBDB.cc\
MNS/ParmTable.cc



#bin_PROGRAMS = BlackBoardDemo fillsqldb
bin_PROGRAMS = parmdb BBS3

parmdb_SOURCES = MNS/parmdb.cc
parmdb_LDADD = libbbs3.la
parmdb_DEPENDENCIES = libbbs3.la $(LOFAR_DEPEND)

BBS3_SOURCES = BBS3.cc
BBS3_LDADD = libbbs3.la
BBS3_DEPENDENCIES = libbbs3.la $(LOFAR_DEPEND)

# BlackBoardDemo_SOURCES = BlackBoardDemo.cc
# BlackBoardDemo_LDADD = libbbs3.la
# BlackBoardDemo_DEPENDENCIES = libbbs3.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common
