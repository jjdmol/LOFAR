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
	Query.h \
	TPersistentObject.h \
	Query/BinaryExprNode.h \
	Query/ColumnExprNode.h \
	Query/ConstExprNode.h \
	Query/Expr.h \
	Query/ExprNode.h \
	Query/FunctionExprNode.h \
	Query/LogicalBinaryExprNode.h \
	Query/LogicalUnaryExprNode.h \
	Query/SQLExprNode.h \
	Query/UnaryExprNode.h

NOINSTHDRS = 
	ObjectCache.h \
	SQLTimeStamp.h

TCCHDRS = \
	PersistenceBroker.tcc \
	TPersistentObject.tcc

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES = libpl.la

libpl_la_SOURCES = \
	Attrib.cc \
	ConnectionHandler.cc \
	DBRepMeta.cc \
	ObjectCache.cc \
	ObjectId.cc \
	Query.cc \
	PersistenceBroker.cc \
	PersistentObject.cc \
	TPersistentObject.cc \
	Query/BinaryExprNode.cc \
	Query/ColumnExprNode.cc \
	Query/ConstExprNode.cc \
	Query/Expr.cc \
	Query/ExprNode.cc \
	Query/LogicalBinaryExprNode.cc \
	Query/LogicalUnaryExprNode.cc \
	Query/SQLExprNode.cc \
	Query/UnaryExprNode.cc

include $(lofar_sharedir)/Makefile.common
