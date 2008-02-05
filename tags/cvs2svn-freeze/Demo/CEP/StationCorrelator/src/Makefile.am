INSTHDRS =						\
StationCorrelator.h					\
StationCorrelatorController.h				\
DH_RSP.h						\
DH_RSPSync.h						\
DH_Empty.h						\
DH_StationData.h					\
DH_CorrCube.h						\
DH_Vis.h						\
WH_RSP.h                                                \
WH_RSPBoard.h                                           \
WH_RSPInput.h                                           \
WH_Transpose.h						\
WH_Correlator.h						\
WH_Dump.h						\
WH_Plot2MAC.h                                           \
SyncStamp.h                                             \
TH_RSP.h

NOINSTHDRS=

TCCHDRS=

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

StationCorrelator_SOURCES 	=	$(DOCHDRS)	\
DH_RSP.cc						\
DH_RSPSync.cc						\
DH_Empty.cc						\
DH_StationData.cc					\
DH_CorrCube.cc						\
DH_Vis.cc						\
WH_RSP.cc                                               \
WH_RSPBoard.cc                                          \
WH_RSPInput.cc                                          \
WH_Transpose.cc						\
WH_Correlator.cc					\
WH_Dump.cc						\
WH_Plot2MAC.cc                                          \
StationCorrelator.cc					\
StationCorrelatorController.cc				\
StationCorrelatorMain.cc				\
SyncStamp.cc                                            \
TH_RSP.cc

AM_YFLAGS               = -d -p KeyParse
AM_LFLAGS               = -PKeyTokenize
LEX_OUTPUT_ROOT         = lex.KeyTokenize

bin_PROGRAMS = StationCorrelator
StationCorrelator_LDADD         =
StationCorrelator_DEPENDENCIES  = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
