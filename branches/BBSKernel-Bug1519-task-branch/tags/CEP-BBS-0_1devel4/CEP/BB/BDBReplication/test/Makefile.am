
check_PROGRAMS	= tBDBReplication

tBDBReplication_SOURCES  	= tBDBReplication.cc

LDADD = ../src/libbdbreplication.la
DEPENDENCIES = ../src/libbdbreplication.la $(LOFAR_DEPEND)


TESTS   =                                       \
        tBDBReplication.sh


include $(top_srcdir)/Makefile.common
