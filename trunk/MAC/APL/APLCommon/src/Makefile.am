AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

include_HEADERS =

DOCHDRS	=	$(include_HEADERS) \
	suite.h \
	test.h
      
noinst_LTLIBRARIES 	= libaplcommon.la

libaplcommon_la_SOURCES= $(DOCHDRS) \
	suite.cpp \
	test.cpp

BUILT_SOURCES = BeamServer_Protocol.ph

include $(lofar_sharedir)/Makefile.common

