cppflags = -I$(top_srcdir)/CoreComps/SAL/src

check_PROGRAMS 		= echoapp ping

TESTS			= #echoapp ping

echoapp_SOURCES 			= Echo.cc Service.cc Echo_Protocol.cc
echoapp_LDADD					= ../src/libsal.la $(LOFAR_DEPEND)
echoapp_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)
echoapp_CPPFLAGS			= $(cppflags)

ping_SOURCES 					= Ping.cc  Echo_Protocol.cc
ping_LDADD						=  ../src/libsal.la $(LOFAR_DEPEND)
ping_DEPENDENCIES			= ../src/libsal.la $(LOFAR_DEPEND)
ping_CPPFLAGS= $(cppflags)

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
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

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common



