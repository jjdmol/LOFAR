AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

check_PROGRAMS 		= tEventPort tProtocol

tProtocol_SOURCES 		= testprotocol.ph testprotocol.cc \
						  Echo_Protocol.ph Echo_Protocol.cc \
						  tProtocol.cc 
tProtocol_LDADD			= ../src/libmacio.la
tProtocol_DEPENDENCIES	= ../src/libmacio.la $(LOFAR_DEPEND)

tEventPort_SOURCES 		= Echo_Protocol.ph Echo_Protocol.cc \
						  tEventPort.cc
tEventPort_LDADD		= ../src/libmacio.la
tEventPort_DEPENDENCIES	= ../src/libmacio.la $(LOFAR_DEPEND)

BUILT_SOURCES = Echo_Protocol.ph testprotocol.ph

EXTRA_DIST =  tProtocol.log_prop
	
clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common



