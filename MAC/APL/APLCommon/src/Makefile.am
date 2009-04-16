AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO  $<

%.conf: %.conf.in
	cp $< $@

%: %.sh.in
	cp $< $@

lib_LTLIBRARIES = libaplcommon.la

libaplcommon_la_CPPFLAGS =  -DBOOST_DISABLE_THREADS \
							-Wno-deprecated \
							-fmessage-length=0 \
							-fdiagnostics-show-location=once

libaplcommon_la_SOURCES   = Package__Version.cc \
							StartDaemon_Protocol.cc \
							Controller_Protocol.cc \
							APLUtilities.cc \
							ControllerDefines.cc \
							CTState.cc \
							ControllerAdmin.cc \
							ChildControl.cc \
							ParentControl.cc 


BUILT_SOURCES 			= $(ph_HEADERS) swlevel swlevel.conf

EXTRA_DIST 				= $(BUILT_SOURCES) $(BUILT_SOURCES)

sysconf_DATA			= swlevel.conf 

bin_SCRIPTS				= swlevel

phdir					=$(includedir)/APL/APLCommon

ph_HEADERS 				= StartDaemon_Protocol.ph \
						  Controller_Protocol.ph

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionaplcommon

versionaplcommon_SOURCES      = versionaplcommon.cc
versionaplcommon_LDADD        = libaplcommon.la
versionaplcommon_DEPENDENCIES = libaplcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

