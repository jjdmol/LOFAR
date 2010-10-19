check_PROGRAMS		= tControllerDefines tAPLUtilities tbitsetUtil tOutOfBand tAntennaMapper tAntennaSet tAntennaPos
AM_CPPFLAGS			= -I../src/

tControllerDefines_SOURCES	= tControllerDefines.cc
tControllerDefines_LDADD	= ../src/libaplcommon.la
tControllerDefines_DEPENDENCIES= ../src/libaplcommon.la $(LOFAR_DEPEND)

tAPLUtilities_SOURCES		= tAPLUtilities.cc
tAPLUtilities_LDADD			= ../src/libaplcommon.la
tAPLUtilities_DEPENDENCIES	= ../src/libaplcommon.la $(LOFAR_DEPEND)

tbitsetUtil_SOURCES			= tbitsetUtil.cc
tbitsetUtil_LDADD			= ../src/libaplcommon.la
tbitsetUtil_DEPENDENCIES	= ../src/libaplcommon.la $(LOFAR_DEPEND)

tOutOfBand_SOURCES			= tOutOfBand.cc
tOutOfBand_LDADD			= ../src/libaplcommon.la
tOutOfBand_DEPENDENCIES		= ../src/libaplcommon.la $(LOFAR_DEPEND)

tAntennaMapper_SOURCES		= tAntennaMapper.cc
tAntennaMapper_LDADD		= ../src/libaplcommon.la
tAntennaMapper_DEPENDENCIES	= ../src/libaplcommon.la $(LOFAR_DEPEND)

tAntennaSet_SOURCES			= tAntennaSet.cc
tAntennaSet_LDADD			= ../src/libaplcommon.la
tAntennaSet_DEPENDENCIES	= ../src/libaplcommon.la $(LOFAR_DEPEND)

tAntennaPos_SOURCES			= tAntennaPos.cc
tAntennaPos_LDADD			= ../src/libaplcommon.la
tAntennaPos_DEPENDENCIES	= ../src/libaplcommon.la $(LOFAR_DEPEND)

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
