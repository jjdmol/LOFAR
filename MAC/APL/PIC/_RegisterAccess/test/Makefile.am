noinst_PROGRAMS 		= aratest

TESTS			= #aratest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

aratest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_builddir)/PIC/RegisterAccess/src \
	-fmessage-length=0

aratest_SOURCES 		= \
	ARATestAnswer.cc \
	ARATest.cc \
	ARATestTask.cc \
	ARATestMain.cc \
	$(top_builddir)/PIC/RegisterAccess/src/ARATest_Protocol.cc
	
aratest_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la
aratest_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
    $(top_builddir)/PIC/RegisterAccess/src/ARATest_Protocol.ph \
	aratest.ns \
	aratest.top \
	log4cplus.properties	

EXTRA_DIST = \
    $(top_builddir)/PIC/RegisterAccess/src/ARATest_Protocol.ph \
	aratest.ns \
	aratest.top \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
