INSTHDRS         	=

NOINSTHDRS 		=

TCCHDRS 		=

AM_CPPFLAGS		= $(EXTRA_CPPFLAGS)

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 				= generator udp_copy

generator_SOURCES			= $(DOCHDRS) \
generator.c

udp_copy_SOURCES			= $(DOCHDRS) \
udp-copy.c

include $(top_srcdir)/Makefile.common
