check_PROGRAMS			= tObservation tStationInfo

TESTSCRIPTS			= tObservation.sh 

TESTS				= $(TESTSCRIPTS)

tObservation_SOURCES		= tObservation.cc
tObservation_LDADD		= ../src/libapplcommon.la
tObservation_DEPENCIES		= ../src/libapplcommon.la $(LOFAR_DEPEND)

tStationInfo_SOURCES		= tStationInfo.cc
tStationInfo_LDADD		= ../src/libapplcommon.la $(LOFAR_DEPEND)
tStationInfo_DEPENCIES		= ../src/libapplcommon.la $(LOFAR_DEPEND)

EXTRA_DIST			= tObservation.log_prop tObservation.run \
				tObservation.stdout \
				tObservation.in_conflict1 \
				tObservation.in_conflict2 \
				tObservation.in_conflict3 \
				tObservation.in_conflict4 \
				tObservation.in_conflict5 \
				tObservation.in_parset1 \
				  $(TESTSCRIPTS)

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)


%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
