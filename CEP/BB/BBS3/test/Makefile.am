check_PROGRAMS = tBBS3 tPredSolv tPerturbed t3C343

tBBS3_SOURCES = tBBS3.cc
tBBS3_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPredSolv_SOURCES = tPredSolv.cc
tPredSolv_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPerturbed_SOURCES = tPerturbed.cc
tPerturbed_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

t3C343_SOURCES = t3C343.cc
t3C343_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)


LDADD				= ../src/libbbs3.la 

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  MPIBIN=$(MPIBIN)

# TESTS = tBBS3_test.sh
TESTS = tPredSolv.sh tPerturbed.sh

include $(lofar_sharedir)/Makefile.common
