AM_CPPFLAGS = -DUSES_NEW_IOSTREAMS
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/APL/RTDBCommon

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionrtdbcommon

INSTHDRS					= CM_Protocol.ph 
NOINSTHDRS					=    
BUILT_SOURCES 				= CM_Protocol.cc \
							  CM_Protocol.ph

pkgincludedir				= $(includedir)/APL/RTDBCommon
pkginclude_HEADERS			= $(NOINSTHDRS) $(INSTHDRS)
DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)
EXTRA_DIST 					= $(sysconf_DATA)

# libraries
lib_LTLIBRARIES 			= librtdbcommon.la

librtdbcommon_la_SOURCES	= Package__Version.cc \
							  $(BUILT_SOURCES) \
							  RTDButilities.cc \
							  ClaimMgrTask.cc 

# version stuff
versionrtdbcommon_SOURCES      = versionrtdbcommon.cc
versionrtdbcommon_LDADD        = librtdbcommon.la
versionrtdbcommon_DEPENDENCIES = librtdbcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

