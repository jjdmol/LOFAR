bin_PROGRAMS = VirtualInstrument

VirtualInstrument_CPPFLAGS = \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

VirtualInstrument_SOURCES = \
	VirtualInstrument.cc \
	MaintenanceVI.cc \
	ObservationVI.cc \
	VirtualInstrumentStartDaemonMain.cc

VirtualInstrument_LDADD = $(LOFAR_DEPEND)
VirtualInstrument_DEPENDENCIES = $(LOFAR_DEPEND)

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS =       $(pkginclude_HEADERS)

include_HEADERS =

BUILT_SOURCES =         \
	VirtualInstrument.conf \
	VirtualInstrument.log_prop

EXTRA_DIST = $(BUILT_SOURCES) $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA =

sysconf_DATA= $(BUILT_SOURCES)

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
