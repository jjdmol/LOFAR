INSTHDRS = \
	Attrib.h \
	Collection.h \
	ConnectionHandler.h \
	DTLBase.h \
	Exception.h \
	ObjectCache.h \
	ObjecId.h \
	PersistenceBroker.h \
	PersistentObject.h \
	Query.h \
	SQLTimeStamp.h \
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

TCCHDRS = \
	PersistenceBroker.tcc \
	TPersistentObject.tcc

DOCHDRS = \
	$(INSTHDRS) \
	$(NOINSTHDRS)

lib_LTLIBRARIES = \
	libpl.la

libpl_la_SOURCES = $(DOCHDRS) \
	Attrib.cc \
	ConnectionHandler.cc \
	ObjectCache.cc \
	ObjectId.cc \
	Query.cc \
	PersistenceBroker.cc \
	PersistentObject.cc \
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
