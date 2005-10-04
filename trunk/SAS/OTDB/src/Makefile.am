INSTHDRS			= OTDBconnection.h	\
					  OTDBconstants.h	\
					  Converter.h \
					  ParamTypeConv.h \
					  UnitConv.h \
					  TreeTypeConv.h \
					  TreeStateConv.h \
					  ClassifConv.h \
					  misc.h \
					  OTDBtypes.h	\
					  OTDBtree.h	\
					  OTDBnode.h	\
					  OTDBvalue.h	\
					  OTDBparam.h	\
					  VICnodeDef.h \
					  TreeMaintenance.h \
					  TreeValue.h 

#					  OTDBaction.h	\
#					  OTDBactionList.h	\
#					  OTDBevent.h	\
#					  OTDBeventList.h	

NOINSTHDRS			= wSpaceSplit.h

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS					= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libotdb.la

libotdb_la_SOURCES 		= $(DOCHDRS) \
							OTDBconnection.cc	\
							misc.cc \
							OTDBtree.cc	\
							OTDBnode.cc	\
							OTDBvalue.cc	\
							OTDBparam.cc	\
							VICnodeDef.cc \
							Converter.cc \
							wSpaceSplit.cc \
							TreeMaintenance.cc \
							TreeValue.cc \
							loadCompFile.cc

#							OTDBaction.cc	\
#						    OTDBactionList.cc	\
#						    OTDBevent.cc	\
#						    OTDBeventList.cc	

include $(top_srcdir)/Makefile.common
