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
	ARATestTask.cc \
	ARATestDriverTask.cc \
	ARATestMain.cc
	
BUILT_SOURCES = \
	aratest.conf \
	log4cplus.properties
	
EXTRA_DIST = \
	aratest.conf \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
