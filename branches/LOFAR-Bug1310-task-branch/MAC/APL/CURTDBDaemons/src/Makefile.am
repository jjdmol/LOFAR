lib_LTLIBRARIES = libcurtdbdaemons.la
libcurtdbdaemons_la_SOURCES = Package__Version.cc

pkginclude_HEADERS = Package__Version.h

SUBDIRS = LoggingProcessor KeyValueLogger SASGateway SoftwareMonitor

bin_PROGRAMS = versioncurtdbdaemons

versioncurtdbdaemons_SOURCES      = versioncurtdbdaemons.cc
versioncurtdbdaemons_LDADD        = libcurtdbdaemons.la
versioncurtdbdaemons_DEPENDENCIES = libcurtdbdaemons.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

