check_PROGRAMS		= tCableAttenuation tRCUCables 

TESTSCRIPTS			= tCableAttenuation.sh tRCUCables.sh

TESTS				= $(TESTSCRIPTS)

tCableAttenuation_SOURCES	= tCableAttenuation.cc ../src/CableAttenuation.cc
tCableAttenuation_LDADD		= $(LOFAR_DEPEND)
tCableAttenuation_DEPENCIES	= $(LOFAR_DEPEND)

tRCUCables_SOURCES			= tRCUCables.cc ../src/RCUCables.cc ../src/CableAttenuation.cc
tRCUCables_LDADD			= $(LOFAR_DEPEND)
tRCUCables_DEPENCIES		= $(LOFAR_DEPEND)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

# TESTS = \
# 	RSPTest \
# 	EPAStub

sbin_PROGRAMS = EPAStub RSPTest

sbin_SCRIPTS = beamtest 

RSPTest_SOURCES = \
	RSPTest.h \
	RSPTest.cc

EPAStub_SOURCES = \
	EPAStub.h \
	EPAStub.cc

EPAStub_LDADD = \
	$(top_builddir)/src/libepa.la

BUILT_SOURCES = \
	RSPTest.conf \
	EPAStub.conf \
	EPAStub.log_prop

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	EPAStub.conf \
	RSPTest.conf \
	EPAStub.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
