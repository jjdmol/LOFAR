AM_CPPFLAGS = -I$(top_srcdir)/RTDB/src

check_PROGRAMS 					= tPropertySet tRTDBPerformance tDPservice testQuery tPerformanceReport

TESTS							= tPropertySet tDPservice

tPropertySet_SOURCES 			= $(BUILT_SOURCES) tPropertySet.cc
tPropertySet_LDADD				= ../src/libgcfrtdb.la $(LOFAR_DEPEND)
tPropertySet_DEPENDENCIES		= ../src/libgcfrtdb.la $(LOFAR_DEPEND)

tRTDBPerformance_SOURCES 		= $(BUILT_SOURCES) tRTDBPerformance.cc RTDBPerfResp.cc
tRTDBPerformance_LDADD			= ../src/libgcfrtdb.la $(LOFAR_DEPEND)
tRTDBPerformance_DEPENDENCIES	= ../src/libgcfrtdb.la $(LOFAR_DEPEND)

tDPservice_SOURCES 				= $(BUILT_SOURCES) tDPservice.cc DPresponse.cc
tDPservice_LDADD				= ../src/libgcfrtdb.la $(LOFAR_DEPEND)
tDPservice_DEPENDENCIES			= ../src/libgcfrtdb.la $(LOFAR_DEPEND)

testQuery_SOURCES 				= $(BUILT_SOURCES) testQuery.cc
testQuery_LDADD					= ../src/libgcfrtdb.la $(LOFAR_DEPEND)
testQuery_DEPENDENCIES			= ../src/libgcfrtdb.la $(LOFAR_DEPEND)

tPerformanceReport_SOURCES 		= $(BUILT_SOURCES) tPerformanceReport.cc RTDBPerfResp.cc
tPerformanceReport_LDADD		= ../src/libgcfrtdb.la $(LOFAR_DEPEND)
tPerformanceReport_DEPENDENCIES	= ../src/libgcfrtdb.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



