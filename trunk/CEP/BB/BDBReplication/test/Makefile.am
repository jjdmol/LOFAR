common_top_builddir=@common_top_builddir@

check_PROGRAMS	= repQuote

repQuote_SOURCES  	= tBDBReplication.cc

LDADD = ../src/libBDBReplication.la
DEPENDENCIES = ../src/libBDBReplication.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
