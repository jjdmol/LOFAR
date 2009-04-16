check_PROGRAMS			= tObservation tCableAttenuation tRCUCables tStationInfo

TESTSCRIPTS			= tObservation.sh tCableAttenuation.sh tRCUCables.sh

TESTS				= $(TESTSCRIPTS)

tObservation_SOURCES		= tObservation.cc
tObservation_LDADD			= ../src/libapplcommon.la
tObservation_DEPENCIES		= ../src/libapplcommon.la $(LOFAR_DEPEND)

tCableAttenuation_SOURCES	= tCableAttenuation.cc
tCableAttenuation_LDADD		= ../src/libapplcommon.la $(LOFAR_DEPEND)
tCableAttenuation_DEPENCIES	= ../src/libapplcommon.la $(LOFAR_DEPEND)

tRCUCables_SOURCES			= tRCUCables.cc
tRCUCables_LDADD			= ../src/libapplcommon.la $(LOFAR_DEPEND)
tRCUCables_DEPENCIES		= ../src/libapplcommon.la $(LOFAR_DEPEND)

tStationInfo_SOURCES		= tStationInfo.cc
tStationInfo_LDADD			= ../src/libapplcommon.la $(LOFAR_DEPEND)
tStationInfo_DEPENCIES		= ../src/libapplcommon.la $(LOFAR_DEPEND)

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
