INSTHDRS = MSDesc.h

NOINSTHDRS = DH_MSMake.h MSCreate.h

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libbbms.la

libbbms_la_SOURCES 	= \
        DH_MSMake.cc \
        MSCreate.cc \
	MSDesc.cc

bin_PROGRAMS = MSSplit makemsdesc readmsdesc makems

MSSplit_SOURCES = MSSplit.cc
MSSplit_DEPENDENCIES = $(LOFAR_DEPEND)

makemsdesc_SOURCES = makemsdesc.cc
makemsdesc_LDADD   = libbbms.la
makemsdesc_DEPENDENCIES = libbbms.la $(LOFAR_DEPEND)

readmsdesc_SOURCES = readmsdesc.cc
readmsdesc_LDADD   = libbbms.la
readmsdesc_DEPENDENCIES = libbbms.la $(LOFAR_DEPEND)

makems_SOURCES = makems.cc
makems_LDADD   = libbbms.la
makems_DEPENDENCIES = libbbms.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = mssplit.g statconv.g

scriptdir = $(bindir)
dist_script_SCRIPTS = mssplit

include $(top_srcdir)/Makefile.common

