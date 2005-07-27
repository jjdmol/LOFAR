check_PROGRAMS			= tConverter

tConverter_SOURCES		= tConverter.cc
tConverter_LDADD		= ../src/libamcimpl.la
tConverter_DEPENDENCIES		= ../src/libamcimpl.la $(LOFAR_DEPEND)

#tITRF_SOURCES			= tITRF.cc
#tITRF_LDADD			= ../src/libamcimpl.la
#tITRF_DEPENDENCIES		= ../src/libamcimpl.la $(LOFAR_DEPEND)

TESTS				= tConverter.sh


TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT) \
				  AIPSPP=$(AIPSPP)

include $(lofar_sharedir)/Makefile.common
