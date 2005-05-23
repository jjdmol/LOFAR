INSTHDRS			= OTDBconnection.h	\
					  OTDBconstants.h	\
					  OTDBtree.h	\
					  OTDBtypes.h	\
					  PICadmin.h	\
					  VICadmin.h	\
					  OTDBaction.h	\
					  OTDBactionList.h	\
					  OTDBevent.h	\
					  OTDBeventList.h	

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS					= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libotdb.la

libotdb_la_SOURCES 		= $(DOCHDRS) \
							OTDBconnection.cc	\
							OTDBtree.cc	\
							PICadmin.cc	\
							VICadmin.cc		\
							OTDBaction.cc	\
						    OTDBactionList.cc	\
						    OTDBevent.cc	\
						    OTDBeventList.cc	

include $(lofar_sharedir)/Makefile.common
