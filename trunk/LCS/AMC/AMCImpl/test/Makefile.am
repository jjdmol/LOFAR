SUBDIRS				= . AMCServer

check_PROGRAMS			= tConverterImpl

tConverterImpl_SOURCES		= tConverterImpl.cc
tConverterImpl_LDADD		= ../src/libamcimpl.la
tConverterImpl_DEPENDENCIES	= ../src/libamcimpl.la $(LOFAR_DEPEND)

TESTS				= tConverterImpl.sh

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
