SUBDIRS	= .

INSTHDRS =		 			\
MeqDomain.h					\
MeqExpr.h					\
MeqHist.h					\
MeqJonesExpr.h					\
MeqJonesNode.h					\
MeqLofarPoint.h					\
MeqLofarStatSources.h				\
MeqMatrix.h					\
MeqMatrixComplexArr.h				\
MeqMatrixComplexSca.h				\
MeqMatrixRealArr.h				\
MeqMatrixRealSca.h				\
MeqMatrixRep.h					\
MeqMatrixTmp.h					\
MeqParm.h					\
MeqParmPolc.h					\
MeqParmSingle.h					\
MeqPhaseRef.h					\
MeqPointDFT.h					\
MeqPointSource.h				\
MeqPolc.h					\
MeqRequest.h					\
MeqRequestId.h					\
MeqResult.h					\
MeqSourceList.h					\
MeqStatExpr.h					\
MeqStation.h					\
MeqStatSources.h				\
MeqStatUVW.h					\
MeqStation.h					\
MeqStoredParmPolc.h				\
MeqUVWPolc.h					\
MeqWsrtInt.h					\
MeqWsrtPoint.h					\
ParmTable.h ParmTableAIPS.h ParmTableDB.h	\
MeqParmHolder.h TPOParm.h					

NOINSTHDRS = 

TCCHDRS = 

include_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = \
	$(INSTHDRS) \
	$(NOINSTHDRS)

lib_LTLIBRARIES 	= libmns.la

libmns_la_SOURCES 	= $(DOCHDRS)		\
MeqDomain.cc					\
MeqExpr.cc					\
MeqHist.cc					\
MeqJonesExpr.cc					\
MeqJonesNode.cc					\
MeqLofarPoint.cc				\
MeqLofarStatSources.cc				\
MeqMatrix.cc					\
MeqMatrixComplexArr.cc				\
MeqMatrixComplexSca.cc				\
MeqMatrixRealArr.cc				\
MeqMatrixRealSca.cc				\
MeqMatrixRep.cc					\
MeqMatrixTmp.cc					\
MeqParm.cc					\
MeqParmPolc.cc					\
MeqParmSingle.cc				\
MeqPhaseRef.cc					\
MeqPointDFT.cc					\
MeqPointSource.cc				\
MeqPolc.cc					\
MeqRequest.cc					\
MeqResult.cc					\
MeqSourceList.cc				\
MeqStatExpr.cc					\
MeqStation.cc					\
MeqStatSources.cc				\
MeqStatUVW.cc					\
MeqStoredParmPolc.cc				\
MeqUVWPolc.cc					\
MeqWsrtInt.cc					\
MeqWsrtPoint.cc					\
ParmTable.cc ParmTableAIPS.cc ParmTableDB.cc	\
TPOParm.cc					

EXTRA_DIST = 

bin_PROGRAMS 		= parmdb

parmdb_SOURCES		= parmdb.cc
parmdb_LDADD		= ./libmns.la
parmdb_DEPENDENCIES	= ./libmns.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
