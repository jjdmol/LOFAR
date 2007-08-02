INSTHDRS =

pkgincludedir = $(includedir)/APL/APLCommon
pkginclude_HEADERS = \
	APLCommonExceptions.h \
	APL_Defines.h \
	APLUtilities.h \
	ControllerDefines.h \
	StationInfo.h \
	CTState.h \
	ControllerInfo.h \
	ControllerAdmin.h \
	ChildControl.h \
	ParentControl.h \
	Observation.h

NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
