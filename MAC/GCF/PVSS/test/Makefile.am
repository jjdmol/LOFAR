AM_CPPFLAGS = -I$(top_srcdir)/PVSS/src

check_PROGRAMS 		= tPVSSservice tPerformance tPVSSconnect tMonitorState tPVSSinfo

TESTS				= tPVSSservice 

tPVSSservice_SOURCES 		= $(BUILT_SOURCES) tPVSSservice.cc Response.cc
tPVSSservice_LDADD			=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tPVSSservice_DEPENDENCIES	= ../src/libgcfpvss.la $(LOFAR_DEPEND)

tPerformance_SOURCES 		= $(BUILT_SOURCES) tPerformance.cc PerformanceResponse.cc
tPerformance_LDADD			=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tPerformance_DEPENDENCIES	= ../src/libgcfpvss.la $(LOFAR_DEPEND)

tPVSSconnect_SOURCES 		= $(BUILT_SOURCES) tPVSSconnect.cc
tPVSSconnect_LDADD			=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tPVSSconnect_DEPENDENCIES	= ../src/libgcfpvss.la $(LOFAR_DEPEND)

tPVSSinfo_SOURCES 			= $(BUILT_SOURCES) tPVSSinfo.cc Response.cc
tPVSSinfo_LDADD				=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tPVSSinfo_DEPENDENCIES		= ../src/libgcfpvss.la $(LOFAR_DEPEND)

tMonitorState_SOURCES 		= $(BUILT_SOURCES) tMonitorState.cc Response.cc
tMonitorState_LDADD			=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tMonitorState_DEPENDENCIES	= ../src/libgcfpvss.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



