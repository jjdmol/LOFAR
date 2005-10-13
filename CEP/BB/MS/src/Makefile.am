INSTHDRS =

NOINSTHDRS = DH_MSMake.h MSCreate.h

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libbbms.la

libbbms_la_SOURCES 	= \
        DH_MSMake.cc \
        MSCreate.cc

bin_PROGRAMS = MSSplit MSDesc makems

MSSplit_SOURCES = MSSplit.cc
MSSplit_DEPENDENCIES = $(LOFAR_DEPEND)

MSDesc_SOURCES = MSDesc.cc
MSDesc_DEPENDENCIES = $(LOFAR_DEPEND)

makems_SOURCES = makems.cc
makems_LDADD   = libbbms.la
makems_DEPENDENCIES = libbbms.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = mssplit.g statconv.g

scriptdir = $(bindir)
dist_script_SCRIPTS = mssplit

include $(top_srcdir)/Makefile.common

