lib_LTLIBRARIES 	= libarrayreceptorgroup.la

pkginclude_HEADERS = \
	../include/ArrayReceptorGroup/ArrayReceptorGroup.h

libarrayreceptorgroup_la_CPPFLAGS= \
	-I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libarrayreceptorgroup_la_SOURCES 		= \
	ArrayReceptorGroup.cc
	
include_HEADERS = 

BUILT_SOURCES = 	

EXTRA_DIST =   

%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
