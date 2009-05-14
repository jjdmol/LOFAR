# programs to run through supplied checktools
CHECKTOOLPROGS	=	tLofarStMan

check_PROGRAMS	=	$(CHECKTOOLPROGS)

TESTSCRIPTS	= 	tLofarStMan.sh

TESTS		=	$(TESTSCRIPTS)

tLofarStMan_SOURCES		= tLofarStMan.cc

LDADD				= ../src/liblofarstman.la
DEPENDENCIES			= ../src/liblofarstman.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
