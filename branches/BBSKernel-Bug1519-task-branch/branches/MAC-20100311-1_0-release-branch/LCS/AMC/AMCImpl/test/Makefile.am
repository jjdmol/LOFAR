check_PROGRAMS			= tConverter tConverterStress

tConverter_SOURCES		= tConverter.cc
tConverter_LDADD		= ../src/libamcimpl.la
tConverter_DEPENDENCIES		= ../src/libamcimpl.la $(LOFAR_DEPEND)

tConverterStress_SOURCES	= tConverterStress.cc
tConverterStress_LDADD		= ../src/libamcimpl.la
tConverterStress_DEPENDENCIES	= ../src/libamcimpl.la $(LOFAR_DEPEND)

TESTS				= tConverter.sh \
				  tConverterStress.sh 


EXTRA_DIST			= $(TESTS) \
				  tConverter.run \
				  tConverterStress.run \
				  tConverter.stdout

include $(top_srcdir)/Makefile.common
