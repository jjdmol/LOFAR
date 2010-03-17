#check_PROGRAMS = tFillRow tJonesCMul3
check_PROGRAMS = tFillRow

tFillRow_SOURCES = tFillRow.cc
tFillRow_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tJonesCMul3_SOURCES = tJonesCMul3.cc utils.cc
#tJonesCMul3_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

LDADD				= ../src/libbbskernel.la

#TESTS = tFillRow.sh tJonesCMul3.sh
TESTS = tFillRow.sh

EXTRA_DIST = $(TESTS)

# Programs to run through supplied checktools
#CHECKTOOLPROGS = tFillRow tJonesCMul3
CHECKTOOLPROGS = tFillRow

include $(top_srcdir)/Makefile.common
