check_PROGRAMS = tFillRow

tFillRow_SOURCES = tFillRow.cc
tFillRow_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tShift_SOURCES = tShift.cc
#tShift_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tDFT_SOURCES = tDFT.cc
#tDFT_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

LDADD				= ../src/libbbskernel.la 

TESTS = tFillRow.sh

EXTRA_DIST = $(TESTS)

# Programs to run through supplied checktools
CHECKTOOLPROGS = tFillRow

include $(top_srcdir)/Makefile.common
