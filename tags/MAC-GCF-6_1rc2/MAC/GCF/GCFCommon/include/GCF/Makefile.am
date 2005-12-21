INSTHDRS =

pkgincludedir = $(includedir)/GCF
pkginclude_HEADERS = \
	GCF_PValue.h \
	GCF_PVChar.h \
	GCF_PVBool.h \
	GCF_PVDouble.h \
	GCF_PVString.h \
	GCF_PVInteger.h \
	GCF_PVUnsigned.h \
	GCF_PVDynArr.h \
	GCF_PVBlob.h \
	CmdLine.h \
	Utils.h \
	GCF_Defines.h \
	ParameterSet.h \
	Thread.h \
	Mutex.h

NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common

