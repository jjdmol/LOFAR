bin_PROGRAMS 		= tCoordClient

tCoordClient_SOURCES	= tCoordClient.cc
tCoordClient_LDADD		= ../src/libcoord.la
tCoordClient_DEPENDENCIES 	= ../src/libcoord.la $(LOFAR_DEPEND)

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL

include $(lofar_sharedir)/Makefile.common

