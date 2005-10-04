INSTHDRS         	=\
StationData.h           \
DH_RSP.h		\
WH_Signal.h		\
WH_FakeStation.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_Generator

TFC_Generator_SOURCES	= $(DOCHDRS) \
TFC_Generator_main.cc	\
StationData.cc		\
WH_Signal.cc		\
WH_FakeStation.cc      	\
DH_RSP.cc		\
AH_FakeStation.cc			

configfilesdir=$(bindir)
configfiles_DATA	=	\
TFC_Generator.machinefile	\
TFC_Generator.log_prop		

include $(top_srcdir)/Makefile.common
