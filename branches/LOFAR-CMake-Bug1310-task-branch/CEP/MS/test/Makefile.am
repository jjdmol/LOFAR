check_PROGRAMS		= tMSSplit

# programs to run through supplied checktools
CHECKTOOLPROGS		= tMSSplit

TESTS			= tMSSplit_test.sh tcombinevds.sh

tMSSplit_SOURCES	= tMSSplit.cc
tMSSplit_DEPENDENCIES	= $(LOFAR_DEPEND)

EXTRA_DIST	= tcombinevds.run tcombinevds.stdout \
		  tcombinevds.in_vds1 tcombinevds.in_vds2

include $(top_srcdir)/Makefile.common

