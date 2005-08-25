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
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	Echo_Protocol.ph \
	echoapp.conf \
	ping.conf \
	ServiceBroker.conf

EXTRA_DIST = \
	Echo_Protocol.ph \
	echoapp.conf \
	ping.conf

%.conf: sal-test.conf.in
	ln -sf $< $@

ServiceBroker.conf:	$(bindir)/ServiceBroker.conf
	cp $< $@

clean-local:
	rm -f *.ph

include $(lofar_sharedir)/Makefile.common



