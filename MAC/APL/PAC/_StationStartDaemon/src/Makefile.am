bin_PROGRAMS 		= StationStartDaemon

StationStartDaemon_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

StationStartDaemon_SOURCES 		= \
	StationStartDaemonMain.cc
	
StationStartDaemon_LDADD		    = $(LOFAR_DEPEND)
StationStartDaemon_DEPENDENCIES	= $(LOFAR_DEPEND)
	
include_HEADERS = 

BUILT_SOURCES = 	

EXTRA_DIST =   

sysconf_DATA= \
	StationStartDaemon.conf \
	StationStartDaemon.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
