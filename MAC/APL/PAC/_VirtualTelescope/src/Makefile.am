AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	AVTDefines.h \
	AVTUtilities.h \
	AVTAPCAnswer.h \
	AVTAPCAnswerHandlerInterface.h \
	AVTLogicalDevice.h \
	AVTLogicalDeviceScheduler.h \
	AVTPropertySetAnswer.h \
	AVTPropertySetAnswerHandlerInterface.h \
	AVTStationBeamformer.h \
	AVTVirtualTelescope.h

noinst_LTLIBRARIES 	= libvirtualtelescope.la

libvirtualtelescope_la_SOURCES= $(DOCHDRS) \
	AVTUtilities.cc \
	AVTAPCAnswer.cc \
	AVTLogicalDevice.cc \
	AVTLogicalDeviceScheduler.cc \
	AVTPropertySetAnswer.cc \
	AVTStationBeamformer.cc \
	AVTVirtualTelescope.cc
	
BUILT_SOURCES = LogicalDevice_Protocol.ph

libvirtualtelescope_la_CPPFLAGS= \
	-I$(top_srcdir)/APLCommon/src 

include $(lofar_sharedir)/Makefile.common

