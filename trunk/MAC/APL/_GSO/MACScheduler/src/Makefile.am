bin_PROGRAMS 		= MACScheduler

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

MACScheduler_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DACC_CONFIGURATIONMGR_UNAVAILABLE

MACScheduler_SOURCES 		= \
	MACScheduler.cc \
	MACSchedulerMain.cc \
	SAS_Protocol.cc
	
MACScheduler_LDADD		= 
	
include_HEADERS = 

BUILT_SOURCES = 	\
	SAS_Protocol.ph

EXTRA_DIST =   \
	SAS_Protocol.ph

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = \
	MACScheduler.conf \
	MACScheduler.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
