check_PROGRAMS 		= tPoly tRosenbrock tMeqMatrix tMeqPolc

TESTS			= tPoly tMeqMatrix tMeqPolc

tPoly_SOURCES 		= tPoly.cc 
tPoly_LDADD		= ../src/libmns.la 
tPoly_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)

tRosenbrock_SOURCES 	= tRosenbrock.cc 
tRosenbrock_LDADD	= ../src/libmns.la 
tRosenbrock_DEPENDENCIES= ../src/libmns.la $(LOFAR_DEPEND)

tMeqMatrix_SOURCES 	= tMeqMatrix.cc 
tMeqMatrix_LDADD	= ../src/libmns.la 
tMeqMatrix_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)

tMeqPolc_SOURCES 	= tMeqPolc.cc 
tMeqPolc_LDADD		= ../src/libmns.la 
tMeqPolc_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)

#tuvw_SOURCES 		= tuvw.cc 
#tuvw_LDADD		= ../src/libmns.la 
#tuvw_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
