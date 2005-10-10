INSTHDRS         	=\
StationData.h           \
DH_RSP.h		\
WH_Signal.h		\
WH_FakeStation.h	\
WH_Strip.h		\
WH_Wrap.h		\
AH_FakeStation.h	\
AH_Recorder.h

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= TFC_Generator TFC_Recorder

TFC_Generator_SOURCES	= $(DOCHDRS) \
TFC_Generator_main.cc	\
StationData.cc		\
WH_Signal.cc		\
WH_FakeStation.cc      	\
WH_Strip.cc		\
WH_Wrap.cc		\
DH_RSP.cc		\
AH_FakeStation.cc			

TFC_Recorder_SOURCES	= $(DOCHDRS) \
TFC_Recorder_main.cc	\
StationData.cc		\
WH_Strip.cc		\
WH_Wrap.cc		\
DH_RSP.cc		\
AH_Recorder.cc			

configfilesdir=$(bindir)
configfiles_DATA	=	\
TFC_Generator.machinefile	\
TFC_Generator.log_prop		

include $(top_srcdir)/Makefile.common
