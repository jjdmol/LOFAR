noinst_PROGRAMS 		= \
	BeamletAllocatorTest

bin_PROGRAMS 		= \
	MACScheduleTest

BeamletAllocatorTest_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

BeamletAllocatorTest_SOURCES 		= \
	BeamletAllocatorTest.cc \
	$(top_srcdir)/src/BeamletAllocator.cc
	
BeamletAllocatorTest_LDADD		= 
	
BUILT_SOURCES = BeamletAllocatorTest.log_prop

EXTRA_DIST = BeamletAllocatorTest.log_prop

MACScheduleTest_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

MACScheduleTest_SOURCES 		= \
	MACScheduleMain.cc \
	MACScheduleAnswer.cc \
	MACScheduleTask.cc 
	
MACScheduleTest_LDADD		= 

sysconf_DATA= \
	MACScheduleTest.conf
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

	
include $(lofar_sharedir)/Makefile.common
