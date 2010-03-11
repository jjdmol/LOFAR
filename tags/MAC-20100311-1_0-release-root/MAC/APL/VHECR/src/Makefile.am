lib_LTLIBRARIES 	= libvhecr.la

libvhecr_la_SOURCES 	= Package__Version.cc \
	VHECRTask.cc \
	TBBTrigger.cc \
	TBBReadCmd.cc

libvhecr_la_SOURCES	+= $(backtrace_SRCS)

bin_PROGRAMS = versionvhecr

versionvhecr_SOURCES      = versionvhecr.cc
versionvhecr_LDADD        = libvhecr.la
versionvhecr_DEPENDENCIES = libvhecr.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
