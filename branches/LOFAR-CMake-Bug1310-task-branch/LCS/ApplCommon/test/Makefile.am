check_PROGRAMS			= tObservation

tObservation_SOURCES		= tObservation.cc
tObservation_LDADD		= ../src/libapplcommon.la
tObservation_DEPENCIES		= ../src/libapplcommon.la $(LOFAR_DEPEND)

TESTSCRIPTS			= tObservation.sh


TESTS				= $(TESTSCRIPTS)

EXTRA_DIST			= tObservation.log_prop tObservation.parset \
				  tObservation.stdout \
				  $(TESTSCRIPTS)

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)


%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
