SUBDIRS				= . AMCClient

check_PROGRAMS			= tSkyCoord \
				  tEarthCoord \
				  tTimeCoord

tSkyCoord_SOURCES		= tSkyCoord.cc
tSkyCoord_LDADD			= ../src/libamcbase.la
tSkyCoord_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tEarthCoord_SOURCES		= tEarthCoord.cc
tEarthCoord_LDADD		= ../src/libamcbase.la
tEarthCoord_DEPENDENCIES	= ../src/libamcbase.la $(LOFAR_DEPEND)

tTimeCoord_SOURCES		= tTimeCoord.cc
tTimeCoord_LDADD		= ../src/libamcbase.la
tTimeCoord_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

TESTS				= tSkyCoord.sh \
				  tEarthCoord.sh \
				  tTimeCoord.sh

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
