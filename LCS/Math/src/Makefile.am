INSTHDRS = \
LCSMath.h \
IComplex.h \
ComplexFunc.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libmath.la

libmath_la_SOURCES 	= $(DOCHDRS) \
LCSMath.cc \
IComplex.cc

include $(lofar_sharedir)/Makefile.common
