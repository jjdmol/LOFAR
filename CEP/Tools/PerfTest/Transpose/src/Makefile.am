if HAVE_CORBA
INCLUDES 	= \
	-I$(basesim_top_srcdir)/Corba \
	-I$(basesim_top_builddir)/src/Corba
endif

bin_PROGRAMS	= Transpose

DOCHDRS 	=				\
DH_2DMatrix					\
Transpose.h					\
WH_FillTFMatrix.h					\
WH_Transpose.h

Transpose_SOURCES  	= $(DOCHDRS)		\
DH_2DMatrix.cc					\
Transpose.cc					\
Simulate.cc					\
WH_FillTFMatrix.cc					\
WH_Transpose.cc					

if HAVE_CORBA
Transpose_LDADD = $(basesim_top_builddir)/src/Corba/libth_corba.la
endif


include $(lofar_sharedir)/Makefile.common
