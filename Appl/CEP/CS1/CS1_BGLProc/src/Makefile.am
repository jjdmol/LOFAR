INSTHDRS         	=\
Correlator.h		\
FFT.h			\
FIR.h			\
AH_BGL_Processing.h	\
WH_BGL_Processing.h	\
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
Correlator.S			\
FIR.S				\
FFT.S				\
CS1_BGL_Processing_main.cc 	\
AH_BGL_Processing.cc		\
WH_BGL_Processing.cc		\
TH_ZoidClient.cc

configfilesdir=$(bindir)
configfiles_DATA	=	\
CS1_BGL_Processing.machinefile	\
CS1_BGL_Processing.log_prop	

include $(top_srcdir)/Makefile.common
