check_PROGRAMS	= tMWControl tMWSpec

CHECKTOOLPROGS	= $(CHECK_PROGRAMS)

TESTS		= tMWControl.sh tMWSpec.sh

LDADD		= ../src/libmwcontrol.la 

tMWControl_SOURCES 	= tMWControl.cc \
			PredifferTest.cc SolverTest.cc MWStepTester.cc
tMWControl_DEPENDENCIES = ../src/libmwcontrol.la $(LOFAR_DEPEND)

tMWSpec_SOURCES 	= tMWSpec.cc
tMWSpec_DEPENDENCIES	= ../src/libmwcontrol.la $(LOFAR_DEPEND)

EXTRA_DIST	= MWStepTester.h PredifferTest.h SolverTest.h \
		tMWControl.in tMWControl.in_ms \
		tMWControl.in_mpi_out3 tMWControl.in_mpi_out4 \
		tMWControl.run tMWControl.stdout \
		tMWSpec.in tMWSpec.stdout

include $(top_srcdir)/Makefile.common

