# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

lib_LTLIBRARIES         = librsp.la

librsp_la_SOURCES  		= RSPport.cc

# if HAVE_SHMEM
# librsp_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS			= RSPIOtest
RSPIOtest_SOURCES 		= RSPIOtest.cc
RSPIOtest_LDADD   		= librsp.la
RSPIOtest_DEPENDENCIES  = librsp.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
