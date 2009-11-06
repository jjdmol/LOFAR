lib_LTLIBRARIES           = libplc.la

libplc_la_SOURCES 		  = Package__Version.cc  DH_ProcControl.cc \
							PCCmd.cc \
							ProcessControl.cc  \
							ProcControlComm.cc  \
							ProcControlServer.cc \
							ProcCtrlProxy.cc \
							ProcCtrlCmdLine.cc \
							ProcCtrlRemote.cc \
							ACCmain.cc

bin_PROGRAMS = versionplc

versionplc_SOURCES      = versionplc.cc
versionplc_LDADD        = libplc.la
versionplc_DEPENDENCIES = libplc.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

