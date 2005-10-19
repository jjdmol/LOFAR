INSTHDRS =

pkgincludedir = $(includedir)/GCF/PAL
pkginclude_HEADERS = \
	GCF_ExtProperty.h \
	GCF_Property.h \
	GCF_MyProperty.h \
	GCF_ExtPropertySet.h \
	GCF_PropertySet.h \
	GCF_MyPropertySet.h \
	GCF_Answer.h \
	GCF_PropertyProxy.h \
	GCF_PVSSPort.h \
	GCF_PVSSInfo.h \
	GCF_SysConnGuard.h
	
NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
