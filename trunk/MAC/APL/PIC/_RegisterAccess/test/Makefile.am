noinst_PROGRAMS 		= aratest

TESTS			= #aratest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen -L $(top_builddir)/PAC/RSPDriver/src $<

aratest_CPPFLAGS= \
	-I$(top_srcdir)/PAC/RSPDriver/src \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/PIC/RegisterAccess/src \
	-fmessage-length=0

aratest_SOURCES 		= \
	ARATestAnswer.cc \
	ARATest.cc \
	ARATestTask.cc \
	ARATestDriverTask.cc \
	ARATestMain.cc \
	RSP_Protocol.cc
	
aratest_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la \
	-lrsp \
	-lgcf -lpml -lgcfcommon -lpa -lsal -ltm
	
aratest_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
  $(top_builddir)/PAC/RSPDriver/src/RSP_Protocol.ph \
	log4cplus.properties	

EXTRA_DIST = \
  $(top_builddir)/PAC/RSPDriver/src/RSP_Protocol.ph \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
