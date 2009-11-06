INSTHDRS         	=\
Package__Version.h      \
BandPass.h		\
LocationInfo.h		\
CorrelatorAsm.h		\
FFT_Asm.h		\
FIR_Asm.h		\
InputData.h		\
TransposedData.h	\
FIR.h			\
PPF.h			\
Transpose.h		\
AsyncTranspose.h	\
BeamFormer.h            \
Correlator.h		\
CN_Processing.h	        \
FCNP_ClientStream.h     \
AsyncCommunication.h    \
PencilBeams.h           \
Stokes.h

NOINSTHDRS 		=

TCCHDRS 		=

AM_CPPFLAGS		= $(EXTRA_CPPFLAGS)

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= CN_Processing 

CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)

CN_Processing_SOURCES	= $(DOCHDRS) \
Package__Version.cc             \
BandPass.cc			\
LocationInfo.cc			\
CorrelatorAsm.S			\
FIR_Asm.S			\
FFT_Asm.S			\
FIR.cc				\
Transpose.cc			\
AsyncTranspose.cc		\
BeamFormer.cc                   \
PPF.cc				\
Correlator.cc			\
CN_Processing_main.cc    	\
CN_Processing.cc		\
FCNP_ClientStream.cc            \
AsyncCommunication.cc           \
PencilBeams.cc                  \
Stokes.cc

configfilesdir=$(bindir)
configfiles_DATA	=	\
CN_Processing.machinefile	\
CNProc.log_prop	

include $(top_srcdir)/Makefile.common
