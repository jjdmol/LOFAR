check_PROGRAMS 		= echoapp ping

TESTS			= #echoapp ping

echoapp_SOURCES 		= Echo.cc Echo_Protocol.cc
echoapp_LDADD		= ../src/libtm.la
echoapp_DEPENDENCIES	= ../src/libtm.la $(LOFAR_DEPEND)

ping_SOURCES 		= Ping.cc Echo_Protocol.cc
ping_LDADD		= ../src/libtm.la
ping_DEPENDENCIES	= ../src/libtm.la $(LOFAR_DEPEND)

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



