check_PROGRAMS = \
	tSQLTimeStamp 
#	tSQLTimeStampCxxTest

tSQLTimeStamp_SOURCES = \
	SQLTimeStampTest.cc \
	SQLTimeStampTest.h \
	tSQLTimeStamp.cc

#tSQLTimeStampCxxTest_SOURCES = \
#	SQLTimeStampCxxTest.hh \
#	SQLTimeStampCxxTest.cc

CXXFLAGS += -Wno-unused

TESTS_ENVIRONMENT               = lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
