AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	AVTAPCAnswer.h \
	AVTLogicalDevice.h \
	AVTPropertySetAnswer.h \
	AVTStationBeamformer.h \
	AVTVirtualTelescope.h

noinst_LTLIBRARIES 	= libvirtualtelescope.la

libvirtualtelescope_la_SOURCES= $(DOCHDRS) \
	AVTAPCAnswer.cc \
	AVTLogicalDevice.cc \
	AVTPropertySetAnswer.cc \
	AVTStationBeamformer.cc \
	AVTVirtualTelescope.cc
	
BUILT_SOURCES = LogicalDevice_Protocol.ph

libvirtualtelescope_la_CPPFLAGS= \
	-I$(top_srcdir)/APLCommon/src 

include $(lofar_sharedir)/Makefile.common

