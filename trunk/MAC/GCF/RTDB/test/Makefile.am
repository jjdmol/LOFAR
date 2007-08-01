AM_CPPFLAGS = -I$(top_srcdir)/RTDB/src

check_PROGRAMS 		= tPropertySet tRTDBPerformance

TESTS				= tPropertySet

tPropertySet_SOURCES 		= $(BUILT_SOURCES) tPropertySet.cc
tPropertySet_LDADD			= ../src/librtdb.la $(LOFAR_DEPEND)
tPropertySet_DEPENDENCIES	= ../src/librtdb.la $(LOFAR_DEPEND)

tRTDBPerformance_SOURCES 		= $(BUILT_SOURCES) tRTDBPerformance.cc RTDBPerfResp.cc
tRTDBPerformance_LDADD			= ../src/librtdb.la $(LOFAR_DEPEND)
tRTDBPerformance_DEPENDENCIES	= ../src/librtdb.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



