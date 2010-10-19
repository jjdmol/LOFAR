check_PROGRAMS				= tMarshallBlitz tBlitz

tMarshallBlitz_SOURCES		= tMarshallBlitz.cc
tMarshallBlitz_LDADD		= 
tMarshallBlitz_DEPENCIES	= $(LOFAR_DEPEND)

tBlitz_SOURCES				= tBlitz.cc
tBlitz_LDADD				= 
tBlitz_DEPENCIES			= $(LOFAR_DEPEND)

TESTSCRIPTS					= 


TESTS						= $(TESTSCRIPTS)

EXTRA_DIST					= 

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)


%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
