check_PROGRAMS		  = TestAsynchronous 

TestAsynchronous_SOURCES  	= TestAsynchronous.cc
TestAsynchronous_LDADD	  	= ../src/libAsyncTest.la 
TestAsynchronous_DEPENDENCIES	= ../src/libAsyncTest.la $(LOFAR_DEPEND)


TESTS	=				\
	Asynchronous_test.sh			


include $(top_srcdir)/Makefile.common
