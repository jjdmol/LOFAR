lib_LTLIBRARIES		= libaps.la

libaps_la_SOURCES	= Package__Version.cc  \
	KeyPrefix.cc \
	KVpair.cc \
	ParameterSet.cc \
	ParameterSetImpl.cc

bin_PROGRAMS = getparsetvalue versionaps

getparsetvalue_SOURCES      = getparsetvalue.cc
getparsetvalue_LDADD        = libaps.la
getparsetvalue_DEPENDENCIES = libaps.la $(LOFAR_DEPEND)

versionaps_SOURCES      = versionaps.cc
versionaps_LDADD        = libaps.la
versionaps_DEPENDENCIES = libaps.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

