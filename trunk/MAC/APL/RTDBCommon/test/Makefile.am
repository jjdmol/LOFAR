AM_CPPFLAGS 					= -DUSES_NEW_IOSTREAMS -I../src

check_PROGRAMS 					= tRTDButilities tClaimManager

TESTS							= 

tRTDButilities_SOURCES 			= $(BUILT_SOURCES) tRTDButilities.cc
tRTDButilities_LDADD			= ../src/librtdbcommon.la $(LOFAR_DEPEND)
tRTDButilities_DEPENDENCIES		= ../src/librtdbcommon.la $(LOFAR_DEPEND)

tClaimManager_SOURCES 			= $(BUILT_SOURCES) tClaimManager.cc
tClaimManager_LDADD				= ../src/librtdbcommon.la $(LOFAR_DEPEND)
tClaimManager_DEPENDENCIES		= ../src/librtdbcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common



