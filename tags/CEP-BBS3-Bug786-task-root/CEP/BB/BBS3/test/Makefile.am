check_PROGRAMS = tMeqPolc tMeqParm tPredSolv tPredict tSubtract tCorrect tPerturbed t3C343 tMSData tShift

tMeqPolc_SOURCES = tMeqPolc.cc
tMeqPolc_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tMeqParm_SOURCES = tMeqParm.cc
tMeqParm_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPredSolv_SOURCES = tPredSolv.cc
tPredSolv_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPredict_SOURCES = tPredict.cc
tPredict_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tSubtract_SOURCES = tSubtract.cc
tSubtract_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tCorrect_SOURCES = tCorrect.cc
tCorrect_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tPerturbed_SOURCES = tPerturbed.cc
tPerturbed_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

t3C343_SOURCES = t3C343.cc
t3C343_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tMSData_SOURCES = tMSData.cc
tMSData_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)

tShift_SOURCES = tShift.cc
tShift_DEPENDENCIES = ../src/libbbs3.la $(LOFAR_DEPEND)


LDADD				= ../src/libbbs3.la 

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  MPIBIN=$(MPIBIN) \
				  AIPSPP=$(AIPSPP)

TESTS = tMeqPolc.sh tMeqParm.sh tPredSolv.sh tPredict.sh tSubtract.sh tCorrect.sh tPerturbed.sh tMSData.sh

include $(top_srcdir)/Makefile.common
