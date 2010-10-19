pkgincludedir = $(includedir)/Suite
pkginclude_HEADERS = \
	suite.h \
	test.h

DOCHDRS	=	$(include_HEADERS)
      
lib_LTLIBRARIES 	= libsuite.la

libsuite_la_SOURCES= $(DOCHDRS) \
	suite.cpp \
	test.cpp

include $(lofar_sharedir)/Makefile.common

