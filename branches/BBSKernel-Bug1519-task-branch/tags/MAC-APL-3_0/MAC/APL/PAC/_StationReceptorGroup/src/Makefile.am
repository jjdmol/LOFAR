lib_LTLIBRARIES 	= libstationreceptorgroup.la

pkginclude_HEADERS = \
	../include/StationReceptorGroup/StationReceptorGroup.h

libstationreceptorgroup_la_CPPFLAGS= \
	-I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libstationreceptorgroup_la_SOURCES 		= \
	StationReceptorGroup.cc
	
include_HEADERS = 

BUILT_SOURCES = 	

EXTRA_DIST =   

%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
