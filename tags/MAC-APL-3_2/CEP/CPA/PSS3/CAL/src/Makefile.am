INSTHDRS = MeqCalibraterImpl.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES         = libcal.la

libcal_la_SOURCES       = MeqCalibraterImpl.cc

AM_YFLAGS               = -d -p KeyParse
AM_LFLAGS               = -PKeyTokenize
LEX_OUTPUT_ROOT         = lex.KeyTokenize


bin_PROGRAMS            = event meqcalibrater

event_SOURCES           = GlishEvent.cc

meqcalibrater_SOURCES   = meqcalibrater.cc
meqcalibrater_LDADD     = ./libcal.la
meqcalibrater_DEPENDENCIES = ./libcal.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
