check_PROGRAMS 		= echoapp ping

TESTS			= #echoapp ping

echoapp_SOURCES 		= Echo.cc Service.cc 
echoapp_LDADD		= ../src/libsal.la
echoapp_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)

ping_SOURCES 		= Ping.cc 
ping_LDADD		= 
ping_DEPENDENCIES	= $(LOFAR_DEPEND)

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../GCFCommon/autogen $< && \
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



