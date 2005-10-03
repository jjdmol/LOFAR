lib_LTLIBRARIES 	= libarrayoperations.la

pkginclude_HEADERS = \
	../include/ArrayOperations/ArrayOperations.h

libarrayoperations_la_CPPFLAGS= \
	-I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libarrayoperations_la_SOURCES 		= \
	ArrayOperations.cc
	
include_HEADERS = 

BUILT_SOURCES = 	

EXTRA_DIST =   

%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
