bin_PROGRAMS 		= VirtualInstrument

VirtualInstrument_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

VirtualInstrument_SOURCES 		= \
	VirtualInstrument.cc \
	MaintenanceVI.cc \
	ObservationVI.cc \
	VirtualInstrumentStartDaemonMain.cc
	
VirtualInstrument_LDADD		    = $(LOFAR_DEPEND)
VirtualInstrument_DEPENDENCIES	= $(LOFAR_DEPEND)
	
include_HEADERS = 

BUILT_SOURCES = 	

EXTRA_DIST =   

sysconf_DATA= \
	VirtualInstrument.conf \
	VirtualInstrument.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
