lib_LTLIBRARIES 	= libtinyframe.la

DOCHDRS = \
BaseDataManager.h \
MiniDataManager.h \
ApplicationHolder.h \
WorkHolder.h \
WH_Example.h \
DH_Example.h \
MyExample.h

#bin_PROGRAMS 		= \
# 			tinyCFmemcpy \
# 			CFinput 

# tinyCFmemcpy_SOURCES	= \
# 			WB_Memcpy.cc \
# 			DH_Memcpy.cc \
# 			Memcpy.cc

# CFinput_SOURCES		= \
# 			WH_TFinput.cc \
# 			DH_Memcpy.cc \
# 			CFinput.cc

# tinyCFmemcpy_LDADD 	= \
# 			libtinyframe.la \
# 			libtransport.la 

# CFinput_LDADD		= \
#			libcepframe.la

libtinyframe_la_SOURCES = $(DOCHDRS) \
MiniDataManager.cc \
BaseDataManager.cc \
ApplicationHolder.cc \
WorkHolder.cc \
DH_Example.cc  \
WH_Example.cc \
MyExample.cc

EXTRA_DIST = assay

AM_YFLAGS               = -d -p AppControllerParse
AM_LFLAGS               = -PAppControllerTokenize
LEX_OUTPUT_ROOT         = lex.AppControllerTokenize

include $(lofar_sharedir)/Makefile.common

