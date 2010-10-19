AM_CPPFLAGS 			= -I$(top_srcdir)/PVSS/src

check_PROGRAMS 			= tGCFtypes

TESTS					= tGCFtypes

tGCFtypes_SOURCES 		= $(BUILT_SOURCES) tGCFtypes.cc
tGCFtypes_LDADD			= ../src/libgcfcommon.la $(LOFAR_DEPEND)
tGCFtypes_DEPENDENCIES	= ../src/libgcfcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



