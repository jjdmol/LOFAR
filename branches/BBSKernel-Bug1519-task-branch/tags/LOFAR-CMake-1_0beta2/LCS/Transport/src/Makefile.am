lib_LTLIBRARIES			= libtransport.la 

libtransport_la_DEPENDENCIES	= $(LOFAR_DEPEND)

libtransport_la_SOURCES		= Package__Version.cc  \
	BGLConnection.cc \
	Connection.cc \
	ConnectionPool.cc \
	CSConnection.cc \
	DataBlobExtra.cc \
	DataHolder.cc \
	DH_BlobStreamable.cc \
	TH_Ethernet.cc \
	TH_File.cc \
	TH_Mem.cc \
	TH_MPI.cc \
	TH_ShMem.cc \
	TH_Socket.cc \
	TransportHolder.cc

bin_PROGRAMS = versiontransport

versiontransport_SOURCES      = versiontransport.cc
versiontransport_LDADD        = libtransport.la
versiontransport_DEPENDENCIES = libtransport.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

