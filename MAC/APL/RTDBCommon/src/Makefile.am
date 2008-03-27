DOCHDRS						= RTDButilities.h
   
BUILT_SOURCES 				= 

EXTRA_DIST 					= $(sysconf_DATA)

pkgincludedir				= $(includedir)/APL/RTDBCommon
pkginclude_HEADERS			= $(BUILT_SOURCES)

lib_LTLIBRARIES 			= librtdbcommon.la

librtdbcommon_la_SOURCES	= Package__Version.cc  $(BUILT_SOURCES) \
							  RTDButilities.cc

bin_PROGRAMS = versionrtdbcommon

versionrtdbcommon_SOURCES      = versionrtdbcommon.cc
versionrtdbcommon_LDADD        = librtdbcommon.la
versionrtdbcommon_DEPENDENCIES = librtdbcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

