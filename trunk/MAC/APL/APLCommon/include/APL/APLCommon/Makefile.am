INSTHDRS =

pkgincludedir = $(includedir)/APL/APLCommon
pkginclude_HEADERS = \
	APLCommonExceptions.h \
	APL_Defines.h \
	APLUtilities.h \
	ControllerDefines.h \
	PropertySetAnswer.h \
	PropertySetAnswerHandlerInterface.h  \
	CTState.h \
	ChildControl.h \
	ParentControl.h \
	ResourceAllocator.h 

#	LogicalDeviceFactoryBase.h 
#	LogicalDeviceFactory.h 
#	LogicalDevice.h 
#	SharedLogicalDeviceFactory.h 
#	SingleInstanceLogicalDeviceFactory.h 

NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
