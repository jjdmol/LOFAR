INSTHDRS         	=	\
bitset.h			\
RSPTimeStamp.h          	\
DH_RSPSync.h            	\
DH_Delay.h			\
DH_RSP.h                	\
DH_Subband.h            	\
DH_RFI_Mitigation.h		\
DH_Visibilities.h		\
RectMatrix.h			\
Stub_BGL.h			\
Stub_BGL_Subband.h		\
Stub_BGL_RFI_Mitigation.h	\
Stub_BGL_Visibilities.h		\
Stub_RSP.h			\
Stub_Delay.h			\
CS1_Config.h

NOINSTHDRS 		=

TCCHDRS 		= RectMatrix.tcc

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES	= libcs1_interface.la

libcs1_interface_la_SOURCES  	= \
bitset.cc	          	\
RSPTimeStamp.cc          	\
DH_RSPSync.cc                   \
DH_Delay.cc			\
DH_RSP.cc                	\
DH_Subband.cc                   \
DH_RFI_Mitigation.cc		\
DH_Visibilities.cc		\
Stub_BGL.cc			\
Stub_BGL_Subband.cc		\
Stub_BGL_RFI_Mitigation.cc	\
Stub_BGL_Visibilities.cc	\
Stub_Delay.cc			\
Stub_RSP.cc

configfilesdir=$(bindir)
configfiles_DATA=CS1.cfg

include $(top_srcdir)/Makefile.common
