bin_PROGRAMS = CCUStartDaemon MaintenanceVI VirtualInstrument ObservationVI

#VirtualInstrument_CPPFLAGS = \
#	-DBOOST_DISABLE_THREADS \
#	-Wno-deprecated \
#	-fmessage-length=0 \
#	-fdiagnostics-show-location=once
##  -DUSE_PVSSPORT

CCUStartDaemon_SOURCES 		= CCUStartDaemonMain.cc
CCUStartDaemon_DEPENDENCIES = $(LOFAR_DEPEND)
CCUStartDaemon_LDADD 		= 

MaintenanceVI_SOURCES 		= MaintenanceVI.cc MaintenanceVIMain.cc
MaintenanceVI_DEPENDENCIES = $(LOFAR_DEPEND)
MaintenanceVI_LDADD 		= 

VirtualInstrument_SOURCES 		= VirtualInstrument.cc VirtualInstrumentMain.cc
VirtualInstrument_DEPENDENCIES = $(LOFAR_DEPEND)
VirtualInstrument_LDADD 		= 

ObservationVI_SOURCES 		= ObservationVI.cc ObservationVIMain.cc
ObservationVI_DEPENDENCIES = $(LOFAR_DEPEND)
ObservationVI_LDADD 		= 

NOINSTHDRS = \
	VirtualInstrument.h \
	MaintenanceVI.h \
	ObservationVI.h

INSTHDRS = 

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS = $(pkginclude_HEADERS) $(BUILT_SOURCES)

include_HEADERS =

BUILT_SOURCES = 

EXTRA_DIST = $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA =

sysconf_DATA = \
	CCUStartDaemon.conf \
	CCUStartDaemon.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
