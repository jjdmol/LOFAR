common_top_builddir=@common_top_builddir@

check_PROGRAMS	= repQuote

repQuote_SOURCES  	= tBDBReplication.cc

LDADD = ../src/libbdbreplication.la
DEPENDENCIES = ../src/libbdbreplication.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
