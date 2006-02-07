# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = libcs1_storage.la

libcs1_storage_la_SOURCES =

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libcs1_storage_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS = CS1_Storage

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libcs1_storage.la
#prg_DEPENDENCIES	= libcs1_storage.la $(LOFAR_DEPEND)

CS1_Storage_SOURCES     = \
CS1_Storage_main.cc     \
AH_Storage.cc           \
WH_SubbandWriter.cc     \
MSWriter.cc             \
MSWriterImpl.cc

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

configfilesdir=$(bindir)
configfiles_DATA                =       \
CS1_Storage.machinefile                 \
CS1_Storage.log_prop                    \
typeStruct_TTeraFlopCorrelator.dpl      \
PropertyInterface.conf

include $(top_srcdir)/Makefile.common
