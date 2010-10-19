
lib_LTLIBRARIES 		= libapplcommon.la

libapplcommon_la_SOURCES = Package__Version.cc   Observation.cc

BUILT_SOURCES 			= 

EXTRA_DIST 				= $(BUILT_SOURCES)

sysconf_DATA			= 

bin_SCRIPTS				= 

bin_PROGRAMS = versionapplcommon

versionapplcommon_SOURCES      = versionapplcommon.cc
versionapplcommon_LDADD        = libapplcommon.la
versionapplcommon_DEPENDENCIES = libapplcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

