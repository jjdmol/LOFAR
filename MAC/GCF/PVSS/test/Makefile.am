AM_CPPFLAGS = -I$(top_srcdir)/PVSS/src

check_PROGRAMS 		= tGSAService tGSAPerformance

TESTS				= tGSAService 

tGSAService_SOURCES 		= $(BUILT_SOURCES) tGSAService.cc Service.cc
tGSAService_LDADD			=  ../src/libsal.la $(LOFAR_DEPEND)
tGSAService_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)

tGSAPerformance_SOURCES 	= $(BUILT_SOURCES) tGSAPerformance.cc PerformanceService.cc
tGSAPerformance_LDADD		=  ../src/libsal.la $(LOFAR_DEPEND)
tGSAPerformance_DEPENDENCIES= ../src/libsal.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



