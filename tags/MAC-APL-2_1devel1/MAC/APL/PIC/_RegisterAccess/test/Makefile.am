noinst_PROGRAMS 		= aratest

TESTS			= #aratest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

aratest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/PIC/RegisterAccess/src \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

aratest_SOURCES 		= \
	ARATestAnswer.cc \
	ARATest.cc \
	ARATestTask.cc \
	ARATestDriverTask.cc \
	ARATestMain.cc
	
aratest_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la \
	-lrsp \
	-lgcf 
	
aratest_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
	log4cplus.properties	

EXTRA_DIST = \
	log4cplus.properties
	
%.properties: ../src/%.properties.in
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
