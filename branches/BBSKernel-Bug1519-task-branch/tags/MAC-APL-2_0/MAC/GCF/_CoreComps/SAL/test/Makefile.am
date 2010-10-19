cppflags = -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src

check_PROGRAMS 		= echoapp ping

TESTS			= #echoapp ping

echoapp_SOURCES 		= Echo.cc Service.cc Echo_Protocol.cc
echoapp_LDADD		= ../src/libsal.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la
echoapp_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND) \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la
echoapp_CPPFLAGS= $(cppflags)

ping_SOURCES 		= Ping.cc  Echo_Protocol.cc
ping_LDADD		=  \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la
ping_DEPENDENCIES	= $(LOFAR_DEPEND) \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la
ping_CPPFLAGS= $(cppflags)

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../MACCommon/autogen $< && \
	chmod a+w $@

BUILT_SOURCES = \
	Echo_Protocol.ph \
	tutorial.ns \
	tutorial.top \
	log4cplus.properties	

EXTRA_DIST = \
	Echo_Protocol.ph \
	tutorial.ns \
	tutorial.top \
	log4cplus.properties

MAC_CONFIG = $(top_srcdir)/../Config
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common



