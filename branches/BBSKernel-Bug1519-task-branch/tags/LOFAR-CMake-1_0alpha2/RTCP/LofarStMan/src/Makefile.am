lib_LTLIBRARIES 	= liblofarstman.la

liblofarstman_la_SOURCES 	= Package__Version.cc \
	LofarStMan.cc LofarColumn.cc Register.cc

bin_PROGRAMS = versionlofarstman

versionlofarstman_SOURCES      = versionlofarstman.cc
versionlofarstman_LDADD        = liblofarstman.la
versionlofarstman_DEPENDENCIES = liblofarstman.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
