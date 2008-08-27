lib_LTLIBRARIES		= libmwcontrol.la

libmwcontrol_la_SOURCES	= Package__Version.cc \
MWGlobalSpec.cc \
MWLocalSpec.cc \
MWMultiSpec.cc \
MWParameterHandler.cc \
MWSpec.cc \
MWStrategySpec.cc \
PredifferBBS.cc \
PredifferProxy.cc \
SolverBBS.cc \
SolverProxy.cc

bin_PROGRAMS 		= versionmwcontrol

versionmwcontrol_SOURCES      = versionmwcontrol.cc
versionmwcontrol_LDADD        = libmwcontrol.la
versionmwcontrol_DEPENDENCIES = libmwcontrol.la $(LOFAR_DEPEND)


include $(top_srcdir)/Makefile.common
