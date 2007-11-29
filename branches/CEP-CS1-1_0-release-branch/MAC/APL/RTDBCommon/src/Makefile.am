DOCHDRS						= RTDButilities.h
   
BUILT_SOURCES 				= 

EXTRA_DIST 					= $(sysconf_DATA)

pkgincludedir				= $(includedir)/APL/RTDBCommon
pkginclude_HEADERS			= $(BUILT_SOURCES)

lib_LTLIBRARIES 			= librtdbcommon.la

librtdbcommon_la_SOURCES	= $(BUILT_SOURCES) \
							  RTDButilities.cc

include $(top_srcdir)/Makefile.common

