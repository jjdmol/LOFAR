lib_LTLIBRARIES			= libcs1_interface.la

libcs1_interface_la_SOURCES	= \
	bitset.cc \
	DH_Delay.cc \
	DH_RFI_Mitigation.cc \
	DH_RSP.cc \
	DH_RSPSync.cc \
	DH_Subband.cc \
	DH_Visibilities.cc \
	RSPTimeStamp.cc \
	Stub_BGL.cc \
	Stub_BGL_RFI_Mitigation.cc \
	Stub_BGL_Subband.cc \
	Stub_BGL_Visibilities.cc \
	Stub_Delay.cc \
	Stub_RSP.cc

configfilesdir			= $(bindir)
dist_configfiles_DATA	= CS1.parset

include $(top_srcdir)/Makefile.common
