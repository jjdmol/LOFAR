
AM_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

TESTS = \
	EPAStub

check_PROGRAMS = \
	EPAStub

EPAStub_SOURCES = \
	EPAStub.h \
	EPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libepa.la \
	-lsuite -lgcf

BUILT_SOURCES =

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
