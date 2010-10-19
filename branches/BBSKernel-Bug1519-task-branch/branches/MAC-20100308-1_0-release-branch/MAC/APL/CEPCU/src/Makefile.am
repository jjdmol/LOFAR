pkginclude_HEADERS = Package__Version.h

SUBDIRS = ApplController
if HAVE_PVSS
  SUBDIRS += OnlineControl CEPlogProcessor BGPlogProcessor # OfflineControl 
endif

bin_PROGRAMS = versioncepcu

versioncepcu_SOURCES      = versioncepcu.cc Package__Version.cc
versioncepcu_LDADD        = 
versioncepcu_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

