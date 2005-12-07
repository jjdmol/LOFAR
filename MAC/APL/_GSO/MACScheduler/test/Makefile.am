noinst_PROGRAMS = \
	BeamletAllocatorTest \
	LogicalSegmentAllocatorTest \
	addVI

bin_PROGRAMS = \
	MACScheduleTest

BeamletAllocatorTest_CPPFLAGS = \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

BeamletAllocatorTest_SOURCES = \
	BeamletAllocatorTest.cc \
	$(top_srcdir)/src/BeamletAllocator.cc

BeamletAllocatorTest_LDADD = 

LogicalSegmentAllocatorTest_CPPFLAGS = \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

LogicalSegmentAllocatorTest_SOURCES = \
	LogicalSegmentAllocatorTest.cc \
	$(top_srcdir)/src/LogicalSegmentAllocator.cc

LogicalSegmentAllocatorTest_LDADD = 

addVI_CPPFLAGS = \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

addVI_SOURCES = addVI.cc

addVI_LDADD = 

MACScheduleTest_CPPFLAGS = \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

MACScheduleTest_SOURCES = \
	MACScheduleMain.cc \
	MACScheduleAnswer.cc \
	MACScheduleTask.cc 

BUILT_SOURCES = \
	MACScheduleTest.conf \
	BeamletAllocatorTest.log_prop \
	LogicalSegmentAllocatorTest.log_prop \
	MACScheduleTest.log_prop

NOINSTHDRS = \
	MACScheduleTask.h \
	MACScheduleAnswer.h

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

MACScheduleTest_LDADD = 

sysconf_DATA = \
	MACScheduleTest.conf \
	BeamletAllocatorTest.log_prop \
	LogicalSegmentAllocatorTest.log_prop \
	MACScheduleTest.log_prop

EXTRA_DIST = $(BUILT_SOURCES) $(sysconf_data)

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
