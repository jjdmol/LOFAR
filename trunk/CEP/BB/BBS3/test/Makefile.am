check_PROGRAMS = tMeqPolc tPredSolv tPerturbed t3C343 tMSData

tMeqPolc_SOURCES = tMeqPolc.cc
tMeqPolc_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPredSolv_SOURCES = tPredSolv.cc
tPredSolv_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPerturbed_SOURCES = tPerturbed.cc
tPerturbed_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

t3C343_SOURCES = t3C343.cc
t3C343_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tMSData_SOURCES = tMSData.cc
tMSData_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)


LDADD				= ../src/libbbs3.la 

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  MPIBIN=$(MPIBIN) \
				  AIPSPP=$(AIPSPP)

TESTS = tMeqPolc.sh tPredSolv.sh tPerturbed.sh tMSData.sh

include $(lofar_sharedir)/Makefile.common
