noinst_PROGRAMS 		= avttest

TESTS			= #avttest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

avttest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

avttest_SOURCES 		= \
	AVTTestAnswer.cc \
	AVTTestTask.cc \
	AVTTestMAC2Task.cc \
	AVTTestMain.cc \
	../src/LogicalDevice_Protocol.cc
	
avttest_LDADD		= \
	-labs
	
BUILT_SOURCES = \
	avttest.conf \
  ../src/LogicalDevice_Protocol.ph \
	log4cplus.properties	

EXTRA_DIST = \
	avttest.conf \
  ../LogicalDevice_Protocol.ph \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
