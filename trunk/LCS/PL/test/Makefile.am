check_PROGRAMS		= tCollection	\
			  tTPO		\
			  tObjectId 	\
			  tSQLTimeStamp \
			  SQLTimeStampCxxTest

tCollection_SOURCES	= tCollection.cc
tCollection_LDADD	= ../src/libpl.la
tCollection_DEPENDENCIES= ../src/libpl.la $(LOFAR_DEPEND)

tTPO_SOURCES		= tTPO.cc
tTPO_LDADD		= ../src/libpl.la
tTPO_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

#myClass_SOURCES		= myClass.cc \
#			  MeqParm.cc
#myClass_LDADD		= ../src/libpl.la
#myClass_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)


tObjectId_SOURCES	= tObjectId.cc
tObjectId_LDADD		= ../src/libpl.la
tObjectId_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tSQLTimeStamp_SOURCES   = SQLTimeStampTest.cc	\
			  SQLTimeStampTest.h	\
			  tSQLTimeStamp.cc

SQLTimeStampCxxTest_SOURCES = \
			  SQLTimeStampCxxTest.hh \
			  SQLTimeStampCxxTest.cc

CXXFLAGS += -Wno-unused-parameter

TESTS_ENVIRONMENT	= lofar_sharedir=$(lofar_sharedir)

TESTS 			= tSQLTimeStamp.sh

include $(lofar_sharedir)/Makefile.common
