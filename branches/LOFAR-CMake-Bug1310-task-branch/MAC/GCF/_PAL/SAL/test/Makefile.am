AM_CPPFLAGS = -I$(top_srcdir)/SAL/src

check_PROGRAMS 		= salEcho salPing tGSAService tGSAPerformance

TESTS				= tGSAService #salEcho salPing

salEcho_SOURCES 		= $(BUILT_SOURCES) Echo.cc Service.cc Echo_Protocol.cc
salEcho_LDADD			= ../src/libsal.la $(LOFAR_DEPEND)
salEcho_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)

salPing_SOURCES 		= $(BUILT_SOURCES) Ping.cc  Echo_Protocol.cc
salPing_LDADD			=  ../src/libsal.la $(LOFAR_DEPEND)
salPing_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)

tGSAService_SOURCES 		= $(BUILT_SOURCES) tGSAService.cc Service.cc
tGSAService_LDADD			=  ../src/libsal.la $(LOFAR_DEPEND)
tGSAService_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)

tGSAPerformance_SOURCES 	= $(BUILT_SOURCES) tGSAPerformance.cc PerformanceService.cc
tGSAPerformance_LDADD		=  ../src/libsal.la $(LOFAR_DEPEND)
tGSAPerformance_DEPENDENCIES= ../src/libsal.la $(LOFAR_DEPEND)

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = Echo_Protocol.ph

EXTRA_DIST = $(sysconf_DATA)

sysconf_DATA= \
	salEcho.conf \
	salPing.conf

%.conf: sal-test.conf.in
	ln -sf $< $@

clean-local:
	rm -f *.ph
	
include $(top_srcdir)/Makefile.common



