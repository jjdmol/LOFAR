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
TryOut.h

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
TryOut.cc


bin_PROGRAMS = BlackBoardDemo

BlackBoardDemo_SOURCES = BlackBoardDemo.cc

BlackBoardDemo_LDADD = libpss3.la
BlackBoardDemo_DEPENDENCIES = libpss3.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common
