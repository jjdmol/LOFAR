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
SC_Simple.cc					\
MMap.h						\
Prediffer.h					\
Solver.h					\
ParmData.h					\
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
MNS/ParmTablePGSQL.h \
MNS/ParmTableMySQL.h \
MNS/ParmTableMonet.h \
MNS/ParmTableBDB.h\
MNS/ParmTableSQLHelper.h \
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
Prediffer.cc						\
Solver.cc						\
ParmData.cc						\
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
MNS/ParmTableMySQL.cc \
MNS/ParmTableMonet.cc \
MNS/ParmTablePGSQL.cc \
MNS/ParmTableBDB.cc\
MNS/ParmTableSQLHelper.cc \
MNS/ParmTable.cc



#bin_PROGRAMS = BlackBoardDemo fillsqldb
bin_PROGRAMS = fillsqldb

fillsqldb_SOURCES = MNS/fillsqldb.cc
fillsqldb_LDADD = libbbs3.la
fillsqldb_DEPENDENCIES = libbbs3.la $(LOFAR_DEPEND)

# BlackBoardDemo_SOURCES = BlackBoardDemo.cc
# BlackBoardDemo_LDADD = libbbs3.la
# BlackBoardDemo_DEPENDENCIES = libbbs3.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common
