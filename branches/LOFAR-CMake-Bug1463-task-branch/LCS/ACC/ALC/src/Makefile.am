lib_LTLIBRARIES           = libalc.la

libalc_la_SOURCES 		  = Package__Version.cc  ACCmd.cc \
							DH_ApplControl.cc \
							ApplControlComm.cc \
							ApplControlClient.cc \
							ACSyncClient.cc  \
							ACAsyncClient.cc \
							ApplControlServer.cc 

bin_PROGRAMS = versionalc

versionalc_SOURCES      = versionalc.cc
versionalc_LDADD        = libalc.la
versionalc_DEPENDENCIES = libalc.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

