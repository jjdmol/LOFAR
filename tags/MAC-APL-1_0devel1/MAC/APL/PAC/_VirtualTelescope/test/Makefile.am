noinst_PROGRAMS 		= avttest

TESTS			= #avttest

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

avttest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/PAC/BeamServer/src

avttest_SOURCES 		= \
	AVTTestAnswer.cc \
	AVTTest.cc \
	AVTTestTask.cc \
	AVTTestMain.cc
	
avttest_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la
avttest_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
	avttest.ns \
	avttest.top \
	log4cplus.properties	

EXTRA_DIST = \
	avttest.ns \
	avttest.top \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
