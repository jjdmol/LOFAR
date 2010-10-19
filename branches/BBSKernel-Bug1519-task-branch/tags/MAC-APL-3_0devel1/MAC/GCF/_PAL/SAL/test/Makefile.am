cppflags = -I$(top_srcdir)/SAL/src \
	-I$(top_srcdir)/include

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
	echoapp.conf \
	ping.conf \
	ServiceBroker.conf \
	mac.log_prop	

EXTRA_DIST = \
	Echo_Protocol.ph \
	echoapp.conf \
	ping.conf \
	mac.log_prop

MAC_CONFIG = $(top_srcdir)/../../Config

%.log_prop: $(MAC_CONFIG)/%.log_prop
	cp $< $@

%.conf: sal-test.conf.in
	ln -sf $< $@

ServiceBroker.conf:	$(bindir)/ServiceBroker.conf
	cp $< $@

include $(lofar_sharedir)/Makefile.common



