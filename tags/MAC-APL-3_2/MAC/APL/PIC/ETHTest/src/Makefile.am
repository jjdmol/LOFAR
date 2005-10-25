
#
# Define ENABLE_CAP_NET_RAW to let RSPDriver run as setuid root
# with only cap_net_raw capability enabled.
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/src

INSTALL=install -C

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

BUILT_SOURCES = 
	ETH_test.conf \
	ETH_realtime.conf

# clean-local:
# 	rm -f \
# 	RSP_Protocol.ph \
# 	RSP_Protocol.cc \
# 	EPA_Protocol.ph \
# 	EPA_Protocol.cc

EXTRA_DIST =

sysconf_DATA = \
	ETH_test.conf \
	ETH_realtime.conf

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

bin_PROGRAMS = \
	ETH_test \
	ETH_realtime

# 	TCP_test \
# 	UDP_test

ETH_test_SOURCES = \
	Timer.h \
	MEPData.h \
	MEPData.cc \
	DataEvent.h \
	ETH_test.cc

 ETH_realtime_SOURCES = \
	Timer.h \
	MEPData.h \
	MEPData.cc \
	DataEvent.h \
	ETH_realtime.cc

# TCP_test_SOURCES = \
# 	Timer.h \
# 	TCP_test.cc

# UDP_test_SOURCES = \
# 	Timer.h \
# 	UDP_test.cc

include $(top_srcdir)/Makefile.common
