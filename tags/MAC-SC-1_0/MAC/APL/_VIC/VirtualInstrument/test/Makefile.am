bin_PROGRAMS 		= avitest

TESTS			= #avitest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

avitest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/VIC/VirtualInstrument/src \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

avitest_SOURCES 		= \
	AVITestAnswer.cc \
	AVITestTask.cc \
	AVITestDriverTask.cc \
	AVITestMain.cc
	
BUILT_SOURCES = 
	
EXTRA_DIST =
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = \
	avitest.conf \
	avitest.log_prop
		
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
