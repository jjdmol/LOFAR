noinst_PROGRAMS 		= avttest

TESTS			= #avttest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen -L $(top_builddir)/PAC/BeamServer/src $<

avttest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/PAC/BeamServer/src \
	-fmessage-length=0

avttest_SOURCES 		= \
	AVTTestAnswer.cc \
	AVTTestTask.cc \
	AVTTestMAC2Task.cc \
	AVTTestMain.cc \
	../src/LogicalDevice_Protocol.cc \
	ABS_Protocol.cc
	
avttest_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la
avttest_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
    $(top_builddir)/PAC/BeamServer/src/ABS_Protocol.ph \
    $(top_builddir)/PAC/VirtualTelescope/src/LogicalDevice_Protocol.ph \
	log4cplus.properties	

EXTRA_DIST = \
    $(top_builddir)/PAC/BeamServer/src/ABS_Protocol.ph \
    $(top_builddir)/PAC/VirtualTelescope/src/LogicalDevice_Protocol.ph \
	log4cplus.properties
	
%.properties: ../src/%.properties.in
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
