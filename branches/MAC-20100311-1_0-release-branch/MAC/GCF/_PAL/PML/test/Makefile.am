AM_CPPFLAGS = -I$(top_srcdir)/PML/src

check_PROGRAMS 		= tPAPerformance

TESTS				= 

tPAPerformance_SOURCES 		= $(BUILT_SOURCES) tPAPerformance.cc PerformanceService.cc
tPAPerformance_LDADD		= ../src/libpml.la ../../SAL/src/libsal.la $(LOFAR_DEPEND)
tPAPerformance_DEPENDENCIES	= ../src/libpml.la ../../SAL/src/libsal.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



