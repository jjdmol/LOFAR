INSTHDRS         	=\
Allocator.h		\
CorrelatorAsm.h		\
FFT_Asm.h		\
FIR_Asm.h		\
InputData.h		\
FilteredData.h		\
TransposedData.h	\
CorrelatedData.h	\
FIR.h			\
PPF.h			\
Transpose.h		\
Correlator.h		\
BGL_Processing.h	\
TH_ZoidClient.h

NOINSTHDRS 		=

TCCHDRS 		=

AM_CPPFLAGS		= $(EXTRA_CPPFLAGS)

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= CS1_BGL_Processing 

CCAS			= $(CC)
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)

CS1_BGL_Processing_SOURCES	= $(DOCHDRS) \
Allocator.cc			\
CorrelatorAsm.S			\
FIR_Asm.S			\
FFT_Asm.S			\
FIR.cc				\
Transpose.cc			\
PPF.cc				\
Correlator.cc			\
CS1_BGL_Processing_main.cc 	\
BGL_Processing.cc		\
TH_ZoidClient.cc

configfilesdir=$(bindir)
configfiles_DATA	=	\
CS1_BGL_Processing.machinefile	\
CS1_BGL_Processing.log_prop	

include $(top_srcdir)/Makefile.common
