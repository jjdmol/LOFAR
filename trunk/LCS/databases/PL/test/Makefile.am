check_PROGRAMS		= \
			  tFooBar \
			  tExpr \
			  tAttrib \
			  tA \
			  tC \
			  tTPO \
			  tCollection	\
			  tObjectId
#			  tSQLTimeStamp

tFooBar_SOURCES		= tFooBar.cc
tFooBar_LDADD		= ../src/libpl.la
tFooBar_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tA_SOURCES              = tA.cc
tA_LDADD		= ../src/libpl.la
tA_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)

tC_SOURCES              = tC.cc
tC_LDADD		= ../src/libpl.la
tC_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)

tTPO_SOURCES		= tTPO.cc
tTPO_LDADD		= ../src/libpl.la
tTPO_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tExpr_SOURCES		= tExpr.cc
tExpr_LDADD		= ../src/libpl.la
tExpr_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tAttrib_SOURCES		= tAttrib.cc
tAttrib_LDADD		= ../src/libpl.la
tAttrib_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tCollection_SOURCES	= tCollection.cc
tCollection_LDADD	= ../src/libpl.la
tCollection_DEPENDENCIES= ../src/libpl.la $(LOFAR_DEPEND)

tObjectId_SOURCES	= tObjectId.cc
tObjectId_LDADD		= ../src/libpl.la
tObjectId_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

#tSQLTimeStamp_SOURCES	= SQLTimeStampTest.cc	\
#			  SQLTimeStampTest.h	\
#			  tSQLTimeStamp.cc

#SQLTimeStampCxxTest_SOURCES = \
#			  SQLTimeStampCxxTest.hh \
#			  SQLTimeStampCxxTest.cc

TESTS			= \
			  tExpr.sh
#			  tSQLTimeStamp.sh

TESTS_ENVIRONMENT	= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
