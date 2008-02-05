AM_CPPFLAGS = -I$(top_srcdir)/PVSS/src

check_PROGRAMS 		= tPVSSservice tPerformance

TESTS				= PVSSservice 

tPVSSservice_SOURCES 		= $(BUILT_SOURCES) tPVSSservice.cc Response.cc
tPVSSservice_LDADD			=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tPVSSservice_DEPENDENCIES	= ../src/libgcfpvss.la $(LOFAR_DEPEND)

tPerformance_SOURCES 		= $(BUILT_SOURCES) tPerformance.cc PerformanceResponse.cc
tPerformance_LDADD			=  ../src/libgcfpvss.la $(LOFAR_DEPEND)
tPerformance_DEPENDENCIES	= ../src/libgcfpvss.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



