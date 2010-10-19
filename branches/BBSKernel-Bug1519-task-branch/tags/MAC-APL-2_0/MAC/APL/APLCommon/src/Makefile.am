include_HEADERS =

DOCHDRS	=	$(include_HEADERS) \
	APLInterTaskPort.h \
	suite.h \
	test.h
      
noinst_LTLIBRARIES 	= libaplcommon.la

libaplcommon_la__CPPFLAGS= \
  -Wno-deprecated 

libaplcommon_la_SOURCES= $(DOCHDRS) \
	APLInterTaskPort.cc \
	suite.cpp \
	test.cpp

include $(lofar_sharedir)/Makefile.common

