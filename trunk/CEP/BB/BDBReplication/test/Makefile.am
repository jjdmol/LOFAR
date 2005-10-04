common_top_builddir=@common_top_builddir@

check_PROGRAMS	= tBDBReplication

tBDBReplication_SOURCES  	= tBDBReplication.cc

LDADD = ../src/libbdbreplication.la
DEPENDENCIES = ../src/libbdbreplication.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT               = lofar_sharedir=$(lofar_sharedir)

TESTS   =                                       \
        tBDBReplication.sh


include $(top_srcdir)/Makefile.common
