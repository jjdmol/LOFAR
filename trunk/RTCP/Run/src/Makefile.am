# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

#lib_LTLIBRARIES         = libcs1_run.la

#libcs1_run_la_SOURCES =

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libcs1_run_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS = packetanalysis

# Lines to build a program prg
#prg_SOURCES		= prg.cc
#prg_LDADD		= libcs1_run.la
#prg_DEPENDENCIES	= libcs1_run.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

packetanalysis_SOURCES = packetanalysis.c

pythondir = $(bindir)/Run
dist_python_SCRIPTS = 	\
			Commands.py \
			getBGPStats.py \
			Host_Names.py \
			Hosts.py \
			Parset.py \
			runOLAP.py \
			Run.py \
			scanf.py \
			Sections.py \
			Stations.py \
			BGPStats.sh \
			getStats \
			OLAP.parset

lofarpythondir = $(bindir)/Run/LOFAR
dist_lofarpython_SCRIPTS = 	\
			LOFAR/BGcontrol.py \
			LOFAR/__init__.py \
			LOFAR_Jobs.py \
			LOFAR/Locations.py \
			LOFAR/Logger.py \
			LOFAR/ObservationID.py \
			LOFAR_Parset.py \
			LOFAR/Parset.py \
			LOFAR/Partitions.py \
			LOFAR/Sections.py \
			LOFAR/Stations.py

utilpythondir = $(bindir)/Run/util
dist_utilpython_SCRIPTS = 	\
			util/Aborter.py \
			util/Commands.py \
			util/dateutil.py \
			util/Hosts.py \
			util/__init__.py \
			util/Parset.py \
			util/scanf.py \
			util/tee.py

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
