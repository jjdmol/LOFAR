lib_LTLIBRARIES		= libbbscontrol.la

libbbscontrol_la_SOURCES= \
	BBSStep.cc \
	BBSSingleStep.cc \
	BBSSolveStep.cc \
	BBSMultiStep.cc \
	BBSProcessControl.cc \
	BBSStrategy.cc \
	BBSStructs.cc \
	DH_BBSStep.cc \
	DH_BBSStrategy.cc \
	ParameterSetReader.cc \
	StreamFormatting.cc

bin_PROGRAMS		= BBSControl

BBSControl_SOURCES	= BBSControl.cc
BBSControl_LDADD	= libbbscontrol.la
BBSControl_DEPENDENCIES = libbbscontrol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
