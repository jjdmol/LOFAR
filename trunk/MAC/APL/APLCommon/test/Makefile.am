check_PROGRAMS				= tControllerDefines

tControllerDefines_SOURCES	= tControllerDefines.cc
tControllerDefines_LDADD	= ../src/libaplcommon.la
tControllerDefines_DEPENCIES= ../src/libaplcommon.la $(LOFAR_DEPEND)

TESTSCRIPTS					= tControllerDefines_test.sh

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir) \
							  LOFARROOT=$(LOFARROOT)

TESTS						= $(TESTSCRIPTS)

EXTRA_DIST					= tControllerDefines.log_prop \
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
