noinst_PROGRAMS 		= \
	BeamletAllocatorTest \
	addVI

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

addVI_CPPFLAGS= \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

addVI_SOURCES 		= addVI.cc

addVI_LDADD		= 

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

BUILT_SOURCES = \
	BeamletAllocatorTest.log_prop \
	MACScheduleTest.log_prop

EXTRA_DIST = \
	BeamletAllocatorTest.log_prop \
	MACScheduleTest.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
