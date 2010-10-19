check_PROGRAMS				= tControllerDefines tAPLUtilities tOutOfBand tStationInfo

tControllerDefines_SOURCES	= tControllerDefines.cc
tControllerDefines_LDADD	= ../src/libaplcommon.la
tControllerDefines_DEPENCIES= ../src/libaplcommon.la $(LOFAR_DEPEND)

tAPLUtilities_SOURCES		= tAPLUtilities.cc
tAPLUtilities_LDADD			= ../src/libaplcommon.la
tAPLUtilities_DEPENCIES		= ../src/libaplcommon.la $(LOFAR_DEPEND)

tOutOfBand_SOURCES			= tOutOfBand.cc
tOutOfBand_LDADD			= ../src/libaplcommon.la
tOutOfBand_DEPENCIES		= ../src/libaplcommon.la $(LOFAR_DEPEND)

tStationInfo_SOURCES		= tStationInfo.cc
tStationInfo_LDADD			= ../src/libaplcommon.la
tStationInfo_DEPENCIES		= ../src/libaplcommon.la $(LOFAR_DEPEND)

TESTSCRIPTS					= tControllerDefines_test.sh


TESTS						= $(TESTSCRIPTS)

EXTRA_DIST					= tControllerDefines.log_prop \
							  tAPLUtilities.log_prop \
							  tOutOfBand.log_prop \
							  ResourceAllocatorTest.log_prop \
							  $(TESTSCRIPTS)

#noinst_PROGRAMS = ResourceAllocatorTest

#ResourceAllocatorTest_CPPFLAGS = 
#	-Wno-deprecated 
#	-fmessage-length=0 
#	-fdiagnostics-show-location=once 
#	-DTESTBUILD
#ResourceAllocatorTest_SOURCES = 
#	ResourceAllocatorTest.cc 
#	$(top_srcdir)/src/ResourceAllocator.cc
#ResourceAllocatorTest_LDADD = 

#BUILT_SOURCES = 
#	ResourceAllocatorTest.log_prop

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)


%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
