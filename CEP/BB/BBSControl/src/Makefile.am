lib_LTLIBRARIES		= libbbscontrol.la

libbbscontrol_la_SOURCES= \
	BBSStep.cc \
	BBSSingleStep.cc \
	BBSSolveStep.cc \
	BBSMultiStep.cc \
	BBSProcessControl.cc \
	BBSKernelProcessControl.cc \
	BBSStrategy.cc \
	BBSStructs.cc \
	DH_BBSStep.cc \
	DH_BBSStrategy.cc \
	ParameterSetReader.cc \
	StreamFormatting.cc

bin_PROGRAMS		= BBSControl BBSKernel

BBSControl_SOURCES	= BBSControl.cc
BBSControl_LDADD	= libbbscontrol.la
BBSControl_DEPENDENCIES = libbbscontrol.la $(LOFAR_DEPEND)

BBSKernel_SOURCES 	= BBSKernel.cc
BBSKernel_LDADD 	= libbbscontrol.la
BBSKernel_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
