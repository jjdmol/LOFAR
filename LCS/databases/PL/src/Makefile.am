INSTHDRS = \
	Attrib.h \
	Collection.h \
	ConnectionHandler.h \
	DBRep.h \
	DBRepHolder.h \
	DBRepMeta.h \
	DTLBase.h \
	Exception.h \
	ObjectId.h \
	PersistenceBroker.h \
	PersistentObject.h \
	QueryObject.h \
	TPersistentObject.h \
	Query/BinaryExprNode.h \
	Query/ColumnExprNode.h \
	Query/ConstExprNode.h \
	Query/Expr.h \
	Query/ExprNode.h \
	Query/FunctionExprNode.h \
	Query/SQLExprNode.h \
	Query/UnaryExprNode.h

NOINSTHDRS = 
	ObjectCache.h \
	SQLTimeStamp.h

TCCHDRS = \
	PersistenceBroker.tcc \
	TPersistentObject.tcc

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES = libpl.la

libpl_la_SOURCES = \
	Attrib.cc \
	ConnectionHandler.cc \
	DBRepMeta.cc \
	ObjectCache.cc \
	ObjectId.cc \
	QueryObject.cc \
	PersistenceBroker.cc \
	PersistentObject.cc \
	TPersistentObject.cc \
	Query/BinaryExprNode.cc \
	Query/ColumnExprNode.cc \
	Query/ConstExprNode.cc \
	Query/Expr.cc \
	Query/ExprNode.cc \
	Query/SQLExprNode.cc \
	Query/UnaryExprNode.cc

include $(lofar_sharedir)/Makefile.common
