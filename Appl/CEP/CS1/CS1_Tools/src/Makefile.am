lib_LTLIBRARIES = libcs1_tools.la
libcs1_tools_la_SOURCES = Package__Version.cc

INSTHDRS         	=

NOINSTHDRS 		=

TCCHDRS 		=

AM_CPPFLAGS		= $(EXTRA_CPPFLAGS)

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 				= versioncs1_tools generator udp_copy

generator_SOURCES			= $(DOCHDRS) \
generator.c

udp_copy_SOURCES			= $(DOCHDRS) \
udp-copy.c

versioncs1_tools_SOURCES      = versioncs1_tools.cc
versioncs1_tools_LDADD        = libcs1_tools.la
versioncs1_tools_DEPENDENCIES = libcs1_tools.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
