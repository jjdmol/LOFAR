check_PROGRAMS		  = ShareInOut 

ShareInOut_SOURCES 	  = ShareInOut.cc

LDADD			  = ../src/libInOutTest.la 
DEPENDENCIES		  = ../src/libInOutTest.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT	  = lofar_sharedir=$(lofar_sharedir) \
					   MPIBIN=$(MPIBIN)

TESTS	=				\
	ShareInOut_test.sh			


include $(lofar_sharedir)/Makefile.common
