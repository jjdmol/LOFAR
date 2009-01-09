CHECKTOOLPROGS				= tMarshallBlitz tBlitz tCableAttenuation

check_PROGRAMS				= $(CHECKTOOLPROGS)

TESTSCRIPTS					= tCableAttenuation.sh

TESTS						= $(TESTSCRIPTS)

tMarshallBlitz_SOURCES		= tMarshallBlitz.cc
tMarshallBlitz_LDADD		= 
tMarshallBlitz_DEPENCIES	= $(LOFAR_DEPEND)

tBlitz_SOURCES				= tBlitz.cc
tBlitz_LDADD				= 
tBlitz_DEPENCIES			= $(LOFAR_DEPEND)

tCableAttenuation_SOURCES	= tCableAttenuation.cc
tCableAttenuation_LDADD		= ../src/librtccommon.la $(LOFAR_DEPEND)
tCableAttenuation_DEPENCIES	= $(LOFAR_DEPEND)

EXTRA_DIST					= 

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

include $(top_srcdir)/Makefile.common
