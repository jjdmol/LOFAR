check_PROGRAMS				= tMarshalling 

tMarshalling_SOURCES		= tMarshalling.cc
tMarshalling_LDADD			= 
tMarshalling_DEPENCIES		= $(LOFAR_DEPEND)

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
