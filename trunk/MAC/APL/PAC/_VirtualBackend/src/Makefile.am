INSTALL=/usr/bin/install -C

DOCHDRS	=	 \
	CEPApplicationManager.h \
	VirtualBackendFactory.h \
	VirtualBackendLD.h
   
noinst_LTLIBRARIES 	= libvb.la

libvb_la_SOURCES	= $(DOCHDRS) \
	CEPApplicationManager.cc \
	VirtualBackendLD.cc
	
libvb_la_CPPFLAGS			= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

bin_PROGRAMS   		= VirtualBackend
  
VirtualBackend_SOURCES		  = VBMain.cc
VirtualBackend_LDADD		    = libvb.la $(LOFAR_DEPEND)
VirtualBackend_DEPENDENCIES	= libvb.la $(LOFAR_DEPEND)
VirtualBackend_CPPFLAGS			= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

configfilesdir=$(bindir)
configfiles_DATA= \
	VirtualBackend.conf \
	CEPAppDefault.param
	
%.conf: %.conf.in
	cp $< $@

%.param: %.param.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

