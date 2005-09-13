bin_PROGRAMS 		= avitest

TESTS			= #avitest

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
	
sysconf_DATA= \
	avitest.conf \
	avitest.log_prop
		
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
