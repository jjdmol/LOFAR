noinst_PROGRAMS 		= avttest

TESTS			= #avttest

avttest_SOURCES 		= \
	AVTTest.cc \
	AVTTestTask.cc \
	AVTTestMain.cc
	
avttest_LDADD		= ../src/libvirtualtelescope.la \
	$(top_builddir)/APLCommon/src/libaplcommon.la
avttest_DEPENDENCIES	= ../src/libvirtualtelescope.la $(LOFAR_DEPEND) \
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
