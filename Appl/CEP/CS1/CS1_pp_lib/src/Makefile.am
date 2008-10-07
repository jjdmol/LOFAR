INSTHDRS = \
		FlaggerStatistics.h \
		MsInfo.h \
		MsFile.h \
		ComplexMedianFlagger.h \
		FrequencyFlagger.h \
		ComplexMedianFlagger2.h \
		MADFlagger.h \
		BandpassCorrector.h \
		DataSquasher.h \
		DataBuffer.h \
		RunDetails.h \
		Pipeline.h 

NOINSTHDRS =

TCCHDRS =

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES         = libcs1_pp_lib.la

libcs1_pp_lib_la_SOURCES = Package__Version.cc  \
		FlaggerStatistics.cc \
		MsInfo.cc \
		MsFile.cc \
		ComplexMedianFlagger.cc \
		FrequencyFlagger.cc \
		ComplexMedianFlagger2.cc \
		MADFlagger.cc \
		BandpassCorrector.cc \
		DataSquasher.cc \
		DataBuffer.cc \
		RunDetails.cc \
		Pipeline.cc 


bin_PROGRAMS = versioncs1_idppp CS1_IDPPP

CS1_IDPPP_SOURCES	= IDPPP.cc PipelineProcessControl.cc
CS1_IDPPP_LDADD		= libcs1_pp_lib.la
CS1_IDPPP_DEPENDENCIES	= libcs1_pp_lib.la $(LOFAR_DEPEND)

pythondir = $(bindir)
dist_python_SCRIPTS = 


versioncs1_idppp_SOURCES      = versioncs1_idppp.cc
versioncs1_idppp_LDADD        = libcs1_pp_lib.la
versioncs1_idppp_DEPENDENCIES = libcs1_pp_lib.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
