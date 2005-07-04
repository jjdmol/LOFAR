noinst_PROGRAMS 		= BeamletAllocatorTest

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

%.log_prop: %.log_prop.in
	cp $< $@
	
	
include $(lofar_sharedir)/Makefile.common
