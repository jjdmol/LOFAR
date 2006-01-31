INSTHDRS         	=\
Correlator.h		\
FIR.h			\
AH_BGL_Processing.h	\
WH_BGL_Processing.h	

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= CS1_BGL_Processing 

CCAS			= gcc
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) -B/bgl/BlueLight/ppcfloor/blrts-gnu/powerpc-bgl-blrts-gnu/bin/

CS1_BGL_Processing_SOURCES	= $(DOCHDRS) \
Correlator.S			\
FIR.S				\
CS1_BGL_Processing_main.cc 	\
AH_BGL_Processing.cc		\
WH_BGL_Processing.cc

configfilesdir=$(bindir)
configfiles_DATA	=	\
CS1_BGL_Processing.machinefile	\
CS1_BGL_Processing.log_prop	

include $(top_srcdir)/Makefile.common
