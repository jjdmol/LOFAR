check_PROGRAMS		= tA \
			  tC \
			  tTPO
#			  tCollection	\
#			  tObjectId
#			  tSQLTimeStamp

tA_SOURCES              = tA.cc
tA_LDADD		= ../src/libpl.la
tA_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)

tC_SOURCES              = tC.cc
tC_LDADD		= ../src/libpl.la
tC_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)

tTPO_SOURCES		= tTPO.cc
tTPO_LDADD		= ../src/libpl.la
tTPO_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

#tCollection_SOURCES	= tCollection.cc
#tCollection_LDADD	= ../src/libpl.la
#tCollection_DEPENDENCIES= ../src/libpl.la $(LOFAR_DEPEND)

#tObjectId_SOURCES	= tObjectId.cc
#tObjectId_LDADD		= ../src/libpl.la
#tObjectId_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

#myClass_SOURCES		= myClass.cc \
#			  MeqParm.cc
#myClass_LDADD		= ../src/libpl.la
#myClass_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

#tSQLTimeStamp_SOURCES	= SQLTimeStampTest.cc	\
#			  SQLTimeStampTest.h	\
#			  tSQLTimeStamp.cc

#SQLTimeStampCxxTest_SOURCES = \
#			  SQLTimeStampCxxTest.hh \
#			  SQLTimeStampCxxTest.cc

CXXFLAGS		+= -Wno-unused-parameter

#TESTS			= tSQLTimeStamp.sh

TESTS_ENVIRONMENT	= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
