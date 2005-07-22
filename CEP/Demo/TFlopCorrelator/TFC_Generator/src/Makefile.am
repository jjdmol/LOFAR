INSTHDRS         	=\
StationData.h           \
WH_FakeStation.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_Generator

TFC_Generator_SOURCES	= $(DOCHDRS) \
TFC_Generator_main.cc	\
WH_FakeStation.cc      	\
AH_FakeStation.cc			

include $(lofar_sharedir)/Makefile.common
