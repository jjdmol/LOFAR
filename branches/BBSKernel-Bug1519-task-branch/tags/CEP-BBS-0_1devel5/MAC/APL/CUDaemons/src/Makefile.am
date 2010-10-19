lib_LTLIBRARIES = libcudaemons.la
libcudaemons_la_SOURCES = Package__Version.cc

pkginclude_HEADERS = Package__Version.h

SUBDIRS = CTStartDaemon LoggingClient ServiceBroker

bin_PROGRAMS = versioncudaemons

versioncudaemons_SOURCES      = versioncudaemons.cc
versioncudaemons_LDADD        = libcudaemons.la
versioncudaemons_DEPENDENCIES = libcudaemons.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

