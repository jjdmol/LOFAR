INSTHDRS			= OTDBconnection.h	\
					  OTDBconstants.h	\
					  OTDBauthConstants.h	\
					  OTDBtree.h	\
					  OTDBtypes.h	\
					  PICadmin.h	\
					  VICadmin.h

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libotdb.la

libotdb 		= $(DOCHDRS) \
					OTDBconnection.cc	\
					OTDBtree.cc	\
					PICadmin.cc	\
					VICadmin.cc	

include $(lofar_sharedir)/Makefile.common
