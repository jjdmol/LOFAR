check_PROGRAMS 		= tWSRT tPoly tMeqMatrix tuvw

TESTS			= tWSRT tPoly tMeqMatrix tuvw

tPoly_SOURCES 		= tPoly.cc 
tPoly_LDADD		= ../src/libmns.la 
tPoly_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)

tMeqMatrix_SOURCES 	= tMeqMatrix.cc 
tMeqMatrix_LDADD	= ../src/libmns.la 
tMeqMatrix_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)

tuvw_SOURCES 		= tuvw.cc 
tuvw_LDADD		= ../src/libmns.la 
tuvw_DEPENDENCIES	= ../src/libmns.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common
