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
	SparseSet.cc \
	Stub_BGL.cc \
	Stub_Delay.cc \
	CS1_Parset.cc \
	Stub_RSP.cc

configfilesdir			= $(bindir)
dist_configfiles_DATA	= CS1.parset

include $(top_srcdir)/Makefile.common
